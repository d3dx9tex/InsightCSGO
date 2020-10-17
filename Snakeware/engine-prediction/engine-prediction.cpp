#include "engine-prediction.h"
#include "../options.hpp"
void EnginePrediction::PreStart()
{
	if (Snakeware::g_flVelocityModifer < 1.f)
		*(bool*)((uintptr_t)g_Prediction + 0x24) = true;

	if (g_ClientState->iDeltaTick > 0)
		g_Prediction->Update(g_ClientState->iDeltaTick, true, g_ClientState->nLastCommandAck, g_ClientState->nLastOutgoingCommand + g_ClientState->iChokedCommands);
}

void EnginePrediction::Start(CUserCmd* cmd, C_BasePlayer* local) {

	static auto weapon    = local->m_hActiveWeapon();
	static auto oldorigin = g_LocalPlayer->m_vecOrigin();


	if (weapon && local) {
		Snakeware::flNextSecondaryAttack = weapon->m_flNextSecondaryAttack();
	}

	//g_Prediction->InPrediction() = true;;
	
	unpred_vel = (g_LocalPlayer->m_vecOrigin() - oldorigin) * (1.0 / g_GlobalVars->interval_per_tick);
	oldorigin = g_LocalPlayer->m_vecOrigin();

	unpred_eyepos = g_LocalPlayer->GetEyePos();


	old_vars.curtime   = g_GlobalVars->curtime;
	old_vars.frametime = g_GlobalVars->frametime;
	old_vars.tickcount = g_GlobalVars->tickcount;

	g_GlobalVars->curtime   = TICKS_TO_TIME(local->m_nTickBase());
	g_GlobalVars->frametime =  g_EngineClient->IsPaused() ? 0.f : g_GlobalVars->interval_per_tick;
	g_GlobalVars->tickcount = TIME_TO_TICKS(g_GlobalVars->curtime);

	g_GameMovement->StartTrackPredictionErrors(local);
	g_MoveHelper->SetHost(local);

	

	memset(&data, 0, sizeof(data));

	g_Prediction->SetupMove(local, cmd, g_MoveHelper, &data);
	g_GameMovement->ProcessMovement(local, &data);
	
	unpred_eyepos = g_LocalPlayer->GetEyePos();

	g_Prediction->FinishMove(local, cmd, &data);
	

	if (weapon)
	weapon->UpdateAccuracyPenalty(); 



}

void EnginePrediction::Finish(C_BasePlayer* local) {

	//g_Prediction->InPrediction = false;
	g_GameMovement->FinishTrackPredictionErrors(local);
	g_MoveHelper->SetHost(nullptr);
	g_GameMovement->Reset();

	g_GlobalVars->curtime   = old_vars.curtime;
	g_GlobalVars->frametime = old_vars.frametime;
	g_GlobalVars->tickcount = old_vars.tickcount;
}
// beta shit predict


int EnginePrediction::get_tickbase()
{
	return g_LocalPlayer->m_nTickBase();
}

float EnginePrediction::get_curtime()
{
	return get_tickbase() * g_GlobalVars->interval_per_tick;
}


Vector EnginePrediction::get_unpred_vel() const
{
	return unpred_vel;
}

Vector EnginePrediction::get_pred_vel() const
{
	return pred_vel;
}

Vector EnginePrediction::get_unpred_eyepos() const
{
	return unpred_eyepos;
}
