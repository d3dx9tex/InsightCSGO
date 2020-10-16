#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
class EnginePrediction : public Singleton<EnginePrediction>
{
public:
	void PreStart();
	void Start(CUserCmd* cmd, C_BasePlayer* local);
	void Finish(C_BasePlayer* local);
	CMoveData data;
	int get_tickbase();

	float get_curtime();

	
	Vector get_unpred_vel() const;
	Vector get_pred_vel() const;
	Vector get_unpred_eyepos() const;
	
	struct {
		float curtime, frametime;
		int tickcount, tickbase;
	}old_vars;

	struct Viewmodel {
		float m_flViewmodelCycle;
		float m_flViewmodelAnimTime;
	} StoredViewmodel;
private:
	int32_t TickBase{}, SeqDiff{};
	Vector unpred_vel, unpred_eyepos,pred_vel;
};