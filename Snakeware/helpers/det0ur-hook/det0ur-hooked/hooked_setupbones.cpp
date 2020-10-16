
#include "../detour_hook.h"
#include "../../../options.hpp"
DetourHooks::SetupBonesT DetourHooks::OriginalSetupBones;

bool __fastcall DetourHooks::hkSetupBones (IClientRenderable* ECX, void* edx, matrix3x4_t* Bone2WorldOut, int iMaxBones, int BoneMask, float flTime) {
	
	// onetap cheat v4 

	auto pPlayer = reinterpret_cast <C_BasePlayer*> ((uintptr_t)ECX - 0x4);
	// Based on legendware.
	if (!pPlayer || !pPlayer->IsAlive() || !pPlayer->isPlayer() || !Bone2WorldOut || pPlayer->IsLocalPlayer())
		return OriginalSetupBones(ECX, Bone2WorldOut, iMaxBones, BoneMask, flTime);

	if (Snakeware::bBoneSetup)                                    return OriginalSetupBones(ECX, Bone2WorldOut, iMaxBones, BoneMask, flTime);
	
	if (pPlayer == g_LocalPlayer)                                 return OriginalSetupBones(ECX, Bone2WorldOut, iMaxBones, BoneMask, flTime);

	if (!g_Options.ragebot_enabled || !g_LocalPlayer->IsAlive())  return OriginalSetupBones(ECX, Bone2WorldOut, iMaxBones, BoneMask, flTime);



	if (Bone2WorldOut) {
		if (iMaxBones > MAXSTUDIOBONES)
			iMaxBones = MAXSTUDIOBONES;

		memcpy(Bone2WorldOut, pPlayer->m_CachedBoneData().Base(), iMaxBones * sizeof(matrix3x4_t));
	}

	return true;
}

