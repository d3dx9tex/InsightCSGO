#pragma once

#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../valve_sdk/sdk.hpp"
#include "../../../singleton.hpp"

struct ReturnInfo {
	int           iDamage, iHitgroup, iPenetrationCount;
	bool          bDidPenetrateWall;
	float         flThickness;
	Vector        vecEnd;
	C_BasePlayer* HitEntity;

	ReturnInfo (int damage, int hitgroup, int penetration_count, bool did_penetrate_wall, float thickness, C_BasePlayer* hit_entity) {
		iDamage           = damage;
		iHitgroup         = hitgroup;
		iPenetrationCount = penetration_count;
		bDidPenetrateWall = did_penetrate_wall;
		flThickness       = thickness;
		HitEntity         = hit_entity;
	}
};

class AutoWall : public Singleton < AutoWall > {
    private:
	struct FireBulletData_t {
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		CTraceFilter* m_filter;
		trace_t m_enter_trace;

		float m_thickness;
		float m_current_damage;
		int m_penetration_count;
	};

	void ScaleDamage             (C_BasePlayer* e, CCSWeaponInfo* weapon_info, int hitgroup, float& current_damage);
	bool HandleBulletPenetration (CCSWeaponInfo* info, FireBulletData_t& data, bool extracheck = false, Vector point = Vector(0, 0, 0));
	bool TraceToExit             (trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace);
	void TraceLine               (Vector& start, Vector& end, unsigned int mask, C_BasePlayer* ignore, trace_t* trace);
	void ClipTrace               (Vector& start, Vector& end, C_BasePlayer* e, unsigned int mask, ITraceFilter* filter, trace_t* old_trace);
	bool IsBreakableEntity       (C_BasePlayer* e);

	float HitgroupDamage         (int iHitGroup);

public:
	std::vector<float>  scanned_damage;
	std::vector<Vector> scanned_points;

	void reset() {
		scanned_damage.clear ();
		scanned_points.clear ();
	}

	bool CanHitFloatingPoint(const Vector& point, const Vector& source);
	ReturnInfo Think        (Vector pos, C_BasePlayer* target, int specific_hitgroup = -1);
};
