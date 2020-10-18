
#include "resolver.h"
#include "../../../options.hpp"
#include "../autowall/ragebot-autowall.h"
#include "../ragebot.h"
#include <algorithm>
#include "../../../valve_sdk/interfaces/IGameEventmanager.hpp"
#include "../../event-logger/event-logger.h"

static float ResolvedYaw[65];




float eyeAngleDiff (float destAngle, float srcAngle) {
	float delta = fmodf(destAngle - srcAngle, 360.0f);

	if (destAngle > srcAngle)
	{
		if (delta >= 180)
			delta -= 360;
	}
	else
	{
		if (delta <= -180)
			delta += 360;
	}

	return delta;
}





void Resolver::PreverseSafePoint (C_BasePlayer * pPlayer , int iSafeSide, float flTime ) {

	// Selfcoded by @5N4K3 & @D1spemel
	// Own u and all !

	// Store var's
	const auto flPoses       = pPlayer->m_flPoseParameter();
	const auto angEyeAngles  = pPlayer->m_angEyeAngles();
	const auto vecVelocity   = pPlayer->m_vecVelocity();
	const auto vecOrigin     = pPlayer->m_vecOrigin();
	const auto flDuckAmount  = pPlayer->m_flDuckAmount();
	const auto flSimtime     = pPlayer->m_flSimulationTime();
	const auto fFlags        = pPlayer->m_fFlags();

	pPlayer->ForceBoneRebuilid   ();
	pPlayer->InvalidateBoneCache ();

	pPlayer->GetEffect() |= 8;

	if   (iSafeSide == -1) {

		pPlayer->GetPlayerAnimState()->m_flGoalFeetYaw = Math::NormalizeYaw(pPlayer->m_angEyeAngles().yaw - 60);

		pPlayer->SetupBones (pLeftMatrix, 128, BONE_USED_BY_ANYTHING, flTime);
		// Left matrix

	}
	else if (iSafeSide == 0) {
		pPlayer->GetPlayerAnimState()->m_flGoalFeetYaw = Math::NormalizeYaw(pPlayer->m_angEyeAngles().yaw);
		pPlayer->SetupBones (pMiddleMatrix, 128, BONE_USED_BY_ANYTHING, flTime);
		// Center matrix

	}
	else if (iSafeSide == 1) {
		pPlayer->GetPlayerAnimState()->m_flGoalFeetYaw = Math::NormalizeYaw(pPlayer->m_angEyeAngles().yaw + 60);
		pPlayer->SetupBones (pRightMatrix, 128, BONE_USED_BY_ANYTHING, flTime);
		// Right matrix

	}

	// Restore var's.
	pPlayer->m_vecVelocity() = vecVelocity;
	pPlayer->m_vecOrigin() = vecOrigin;
	pPlayer->m_flDuckAmount() = flDuckAmount;
	pPlayer->m_flSimulationTime() = flSimtime;
	pPlayer->m_angEyeAngles() = angEyeAngles;
	pPlayer->m_fFlags() = fFlags;
	pPlayer->m_vecAbsVelocity() = vecVelocity;

	pPlayer->GetEffect() &= ~8;

}


void StoreStatusPlayer(C_BasePlayer* pPlayer, int resolve_info, int side) {
	Snakeware::Delta = side == -1 ? (resolve_info > 30 ? "Max Left" : "Low Left") : (resolve_info > 30 ? "Max Right" : "Low Right");
}


void Resolver::DetectFakeSide (C_BasePlayer * pPlayer) {

	if (!pPlayer) return;
	auto Index		= pPlayer->EntIndex() - 1;
	auto &rRecord	= ResolveRecord[Index];
	float flYaw		= pPlayer->m_angEyeAngles().yaw;
	int missedshots = 0;

	StoreResolveDelta(pPlayer, &rRecord);

	if (pPlayer->m_fFlags() & FL_ONGROUND) {

		if (pPlayer->m_vecVelocity().Length2D() < 0.15f) {
			auto Delta = eyeAngleDiff(pPlayer->m_angEyeAngles().yaw, pPlayer->GetPlayerAnimState()->m_flGoalFeetYaw);

			if (pPlayer->GetAnimOverlays()[3].m_flWeight == 0.0f && pPlayer->GetAnimOverlays()[3].m_flCycle == 0.0f) {
				rRecord.iResolvingWay = Math::Clamp((2 * (Delta <= 0.f) - 1), -1, 1);
				rRecord.bWasUpdated   = true;
			}
		}
		else {

			float Rate  = abs (pPlayer->GetAnimOverlays()[6].m_flPlaybackRate - rRecord.ResolverLayers[0][6].m_flPlaybackRate);
			float Rate2 = abs (pPlayer->GetAnimOverlays()[6].m_flPlaybackRate - rRecord.ResolverLayers[1][6].m_flPlaybackRate);
			float Rate3 = abs (pPlayer->GetAnimOverlays()[6].m_flPlaybackRate - rRecord.ResolverLayers[2][6].m_flPlaybackRate);

			if (Rate < Rate3 || Rate2 <= Rate3 ||      (int)(float)      (Rate3 * 1000.0f)) {
				if (Rate >= Rate2 && Rate3 > Rate2 && !(int)(float)      (Rate2 * 1000.0f)) {

					rRecord.iResolvingWay = 1;
					rRecord.bWasUpdated   = true;
				}
			}
			else {

				rRecord.iResolvingWay = -1;
				rRecord.bWasUpdated   = true;
			}
		}
	    int		   resolve_value		= (pPlayer->GetAnimOverlay(3)->m_flCycle == 0.f && pPlayer->GetAnimOverlay(3)->m_flWeight == 0.f) ? 60 : 29;
		const auto Choked				= std::max(0, TIME_TO_TICKS(pPlayer->m_flSimulationTime() - pPlayer->m_flOldSimulationTime()) - 1);
		bool       backward				= pPlayer->m_angEyeAngles().yaw > 90 && pPlayer->m_angEyeAngles().yaw < -90;
		if (Choked >= 1 && !GetAsyncKeyState(g_Options.ragebot_force_safepoint)) {

			if (rRecord.iResolvingWay < 0) {

				if (rRecord.iMissedShots != 0) {
					//bruteforce player angle accordingly
					switch (rRecord.iMissedShots % 2) {
					case 1: {
						if (backward) {
							flYaw += resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, 1);

						}
						else {
							flYaw -= resolve_value;
						    StoreStatusPlayer(pPlayer, resolve_value, -1);
						}
					}
					break;

					case 0: {
						if (backward) {
							flYaw -= resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, -1);
						}
						else {
							flYaw += resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, 1);
						}
					}
					break;

					}
				}
				else {
					if (backward) {
						flYaw -= resolve_value;
						StoreStatusPlayer(pPlayer, resolve_value, -1);
					}
					else {
						flYaw += resolve_value;
						StoreStatusPlayer(pPlayer, resolve_value, 1);
					}
				}
			}
			else if (rRecord.iResolvingWay > 0) {

				if (rRecord.iMissedShots != 0) {
					//bruteforce player angle accordingly
					switch (rRecord.iMissedShots % 2) {
					case 1: {
						if (backward) {
							flYaw -= resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, -1);

						}
						else {
							flYaw += resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, 1);
						}
					}
					break;

					case 0: {
						if (backward) {
							flYaw += resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, 1);
						}
						else {
							flYaw -= resolve_value;
							StoreStatusPlayer(pPlayer, resolve_value, -1);
						}
					}
					break;

					}
				}
				else {
					if (backward) {
						flYaw += resolve_value;
						StoreStatusPlayer(pPlayer, resolve_value, 1);
					}
					else {
						flYaw -= resolve_value;
						StoreStatusPlayer(pPlayer, resolve_value, -1);
					}
				}
			}
			

			Math::NormalizeYaw(flYaw						);
			Math::NormalizeYaw(pPlayer->m_angEyeAngles().yaw);
			pPlayer->GetPlayerAnimState()->m_flGoalFeetYaw = flYaw;
		}
		
	}
}




void Resolver::StoreResolveDelta(C_BasePlayer * pPlayer,ResolveInfo * cData) {

	const auto flPoses        = pPlayer->m_flPoseParameter();
	const auto angEyeAngles   = pPlayer->m_angEyeAngles();
	const auto vecVelocity    = pPlayer->m_vecVelocity();
	const auto vecOrigin      = pPlayer->m_vecOrigin();
	const auto flDuckAmount   = pPlayer->m_flDuckAmount();
	const auto flSimtime      = pPlayer->m_flSimulationTime();
	const auto fFlags         = pPlayer->m_fFlags();


	std::memcpy(cData->ResolverLayers[0], pPlayer->GetAnimOverlays(), (sizeof(AnimationLayer) * pPlayer->GetNumAnimOverlays()));

	pPlayer->ForceBoneRebuilid();
	pPlayer->m_vecVelocity()      = vecVelocity;
	pPlayer->m_vecOrigin()        = vecOrigin;
	pPlayer->m_flDuckAmount()     = flDuckAmount;
	pPlayer->m_flSimulationTime() = flSimtime;
	pPlayer->m_angEyeAngles()     = angEyeAngles;
	pPlayer->m_fFlags()           = fFlags;
	pPlayer->m_vecAbsVelocity()   = vecVelocity;


	std::memcpy(cData->ResolverLayers[1], pPlayer->GetAnimOverlays(), (sizeof(AnimationLayer) * pPlayer->GetNumAnimOverlays()));

	pPlayer->ForceBoneRebuilid();
	pPlayer->m_vecVelocity() = vecVelocity;
	pPlayer->m_vecOrigin() = vecOrigin;
	pPlayer->m_flDuckAmount() = flDuckAmount;
	pPlayer->m_flSimulationTime() = flSimtime;
	pPlayer->m_angEyeAngles() = angEyeAngles;
	pPlayer->m_fFlags() = fFlags;
	pPlayer->m_vecAbsVelocity() = vecVelocity;

	std::memcpy(cData->ResolverLayers[2], pPlayer->GetAnimOverlays(), (sizeof(AnimationLayer) * pPlayer->GetNumAnimOverlays()));


	pPlayer->ForceBoneRebuilid();
	pPlayer->m_vecVelocity()      = vecVelocity;
	pPlayer->m_vecOrigin()        = vecOrigin;
	pPlayer->m_flDuckAmount()     = flDuckAmount;
	pPlayer->m_flSimulationTime() = flSimtime;
	pPlayer->m_angEyeAngles()     = angEyeAngles;
	pPlayer->m_fFlags()           = fFlags;
	pPlayer->m_vecAbsVelocity()   = vecVelocity;

}
