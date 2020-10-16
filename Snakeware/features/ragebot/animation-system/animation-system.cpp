#include "animation-system.h"
#include "../../../options.hpp"
#include "../resolver/resolver.h"
#include "../player-log/player-log.h"


void Animations::FakeAnimation()
{
	//static bool ShouldInitAnimstate = false;

	if ((!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame()) || !g_LocalPlayer) {
		return;
	}
	if (!g_Options.chams_fake) return;

	if (!g_LocalPlayer->IsAlive()) {
		return;
	}

	static CBaseHandle* selfhandle = nullptr;
	static float spawntime = g_LocalPlayer->m_flSpawnTime();

	auto alloc = FakeAnimstate == nullptr;
	auto change = !alloc && selfhandle != &g_LocalPlayer->GetRefEHandle();
	auto reset = !alloc && !change && g_LocalPlayer->m_flSpawnTime() != spawntime;

	if (change) {
		memset(&FakeAnimstate, 0, sizeof(FakeAnimstate));
		selfhandle = (CBaseHandle*)&g_LocalPlayer->GetRefEHandle();
	}
	if (reset) {
		g_LocalPlayer->ResetAnimationState(FakeAnimstate);
		spawntime = g_LocalPlayer->m_flSpawnTime();
	}

	if (alloc || change) {
		FakeAnimstate = reinterpret_cast<CCSGOPlayerAnimState*>(g_MemAlloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (FakeAnimstate)
			g_LocalPlayer->CreateAnimationState(FakeAnimstate);
	}

	if (FakeAnimstate->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		FakeAnimstate->m_iLastClientSideAnimationUpdateFramecount -= 1.f;

	g_LocalPlayer->GetEffect() |= 0x8;

	g_LocalPlayer->InvalidateBoneCache();

	AnimationLayer backup_layers[13];
	if (g_LocalPlayer->m_flSimulationTime() != g_LocalPlayer->m_flOldSimulationTime())
	{
		bool pidoras = true;
		std::memcpy(backup_layers, g_LocalPlayer->GetAnimOverlays(),
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));

		g_LocalPlayer->UpdateAnimationState(FakeAnimstate, Snakeware::FakeAngle); // update animstate
		g_LocalPlayer->SetAbsAngles(QAngle(0, FakeAnimstate->m_flGoalFeetYaw, 0));
		g_LocalPlayer->GetAnimOverlay(12)->m_flWeight = FLT_EPSILON;
		g_LocalPlayer->SetupBones(Snakeware::FakeMatrix, 128, 0x7FF00, g_GlobalVars->curtime);// setup matrix

		for (auto& i : Snakeware::FakeMatrix)
		{
			i[0][3] -= g_LocalPlayer->GetRenderOrigin().x;
			i[1][3] -= g_LocalPlayer->GetRenderOrigin().y;
			i[2][3] -= g_LocalPlayer->GetRenderOrigin().z;
		}


		std::memcpy(g_LocalPlayer->GetAnimOverlays(), backup_layers,
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));
	}

	//csgo->animstate = FakeAnimstate; usseles

	g_LocalPlayer->GetEffect() &= ~0x8;
}


bool CanFix() {

	if (g_Options.antihit_enabled)        return true;
	if (g_Options.misc_legit_antihit)     return true;
	if (g_Options.misc_fakelag_ticks > 1) return true;

	return false;
}
void Animations::FixLocalPlayer() {

	auto animstate = g_LocalPlayer->GetPlayerAnimState();
	if (!animstate)
		return;
	if (!g_LocalPlayer->IsAlive()) return;
	if (!g_EngineClient->IsInGame()) return;
	if (!CanFix) return;

	const auto backup_frametime = g_GlobalVars->frametime;
	const auto backup_curtime = g_GlobalVars->curtime;

	animstate->m_flGoalFeetYaw = Snakeware::RealAngle.yaw;

	if (animstate->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		animstate->m_iLastClientSideAnimationUpdateFramecount -= 1.f;

	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->curtime = g_LocalPlayer->m_flSimulationTime();

	g_LocalPlayer->m_iEFlags() &= ~0x1000;
	g_LocalPlayer->m_vecAbsVelocity() = g_LocalPlayer->m_vecVelocity();

	static float angle = animstate->m_flGoalFeetYaw;

	animstate->m_flFeetYawRate = 0.f;

	AnimationLayer backup_layers[13];
	if (g_LocalPlayer->m_flSimulationTime() != g_LocalPlayer->m_flOldSimulationTime())
	{
		std::memcpy(backup_layers, g_LocalPlayer->GetAnimOverlays(),
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));

		g_LocalPlayer->m_bClientSideAnimation() = true;
		g_LocalPlayer->UpdateAnimationState(animstate, Snakeware::FakeAngle);
		g_LocalPlayer->UpdateClientSideAnimation();


		angle = animstate->m_flGoalFeetYaw;

		std::memcpy(g_LocalPlayer->GetAnimOverlays(), backup_layers,
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));
	}

	animstate->m_flGoalFeetYaw = angle;
	g_GlobalVars->curtime = backup_curtime;
	g_GlobalVars->frametime = backup_frametime;

}
void Animations::SetLocalPlayerAnimations()
{
	auto animstate = g_LocalPlayer->GetPlayerAnimState();
	if (!animstate) return;
	// weawe.su reverse
	if (g_EngineClient->IsInGame())
	{
		if (g_Input->m_fCameraInThirdPerson && g_Options.misc_thirdperson)
			g_LocalPlayer->SetSnakewareAngles(Snakeware::FakeAngle);

		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
			animstate->m_bOnGround = true;
			animstate->m_bInHitGroundAnimation = false;
		}

		g_LocalPlayer->SetAbsAngles(QAngle(0, g_LocalPlayer->GetPlayerAnimState()->m_flGoalFeetYaw, 0));
	}

}


