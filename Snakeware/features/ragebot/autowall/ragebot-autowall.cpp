#include "ragebot-autowall.h"
#include "../../../helpers/math.hpp"
#include "../../../valve_sdk/math/Vector.hpp"
#include "../../../options.hpp"
#include "../../../Protected/enginer.h"

// Supreamcy autowall + skeet.idn & otv2 code
// Credit's : @Snake



#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E'
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z'

ReturnInfo AutoWall::Think (Vector pos, C_BasePlayer * target, int specific_hitgroup) {

	ReturnInfo return_info = ReturnInfo(-1, -1, 4, false, 0.f, nullptr);
	C_BasePlayer * Local = g_LocalPlayer;
	Vector start = g_LocalPlayer->GetEyePos();

	FireBulletData_t fire_bullet_data;
	fire_bullet_data.m_start             = start;
	fire_bullet_data.m_end               = pos;
	fire_bullet_data.m_current_position  = start;
	fire_bullet_data.m_thickness         = 0.f;
	fire_bullet_data.m_penetration_count = 4;
	

	Math::AngleVectors(Math::CalcAngle(start, pos), fire_bullet_data.m_direction);

	static const auto filter_simple = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>( (void*)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3d);

	uint32_t dwFilter[4] = { filter_simple, reinterpret_cast<uint32_t>(Local), 0, 0 };

	fire_bullet_data.m_filter = (CTraceFilter*)(dwFilter);

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return return_info;

	auto weapon_info = weapon->GetCSWeaponData();
	if (!weapon_info) return return_info;

	float range = std::min(weapon_info->flRange, (start - pos).Length());

	pos = start + (fire_bullet_data.m_direction * range);
	fire_bullet_data.m_current_damage = weapon_info->iDamage;

	for (int i = 0; i < scanned_points.size(); i++) {
		if (pos == scanned_points[i]) {
			return_info.iDamage = scanned_damage[i];
			return return_info;
		}
	}

	while (fire_bullet_data.m_current_damage > 0 && fire_bullet_data.m_penetration_count > 0)
	{
		return_info.iPenetrationCount = fire_bullet_data.m_penetration_count;

		TraceLine(fire_bullet_data.m_current_position, pos, MASK_SHOT | CONTENTS_GRATE, g_LocalPlayer, &fire_bullet_data.m_enter_trace);
		ClipTrace(fire_bullet_data.m_current_position, fire_bullet_data.m_current_position + (fire_bullet_data.m_direction * 40.f), target, MASK_SHOT | CONTENTS_GRATE, fire_bullet_data.m_filter, &fire_bullet_data.m_enter_trace);

		const float distance_traced = (fire_bullet_data.m_enter_trace.endpos - start).Length();
		fire_bullet_data.m_current_damage *= pow(weapon_info->flRangeModifier, (distance_traced / 500.f));

		if (fire_bullet_data.m_enter_trace.fraction == 1.f)
		{
			if (target && specific_hitgroup != -1)
			{
				ScaleDamage(target, weapon_info, specific_hitgroup, fire_bullet_data.m_current_damage);

				return_info.iDamage = fire_bullet_data.m_current_damage;
				return_info.iHitgroup = specific_hitgroup;
				return_info.vecEnd = fire_bullet_data.m_enter_trace.endpos;
				return_info.HitEntity = target;
			}
			else
			{
				return_info.iDamage = fire_bullet_data.m_current_damage;
				return_info.iHitgroup = -1;
				return_info.vecEnd = fire_bullet_data.m_enter_trace.endpos;
				return_info.HitEntity = nullptr;
			}

			break;
		}

		if (fire_bullet_data.m_enter_trace.hitgroup > 0 && fire_bullet_data.m_enter_trace.hitgroup <= 8)
		{
			if
				(
				(target && fire_bullet_data.m_enter_trace.hit_entity != target)
					||
					(reinterpret_cast<C_BasePlayer*>(fire_bullet_data.m_enter_trace.hit_entity)->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()))
			{
				return_info.iDamage = -1;
				return return_info;
			}

			if (specific_hitgroup != -1)
			{
				ScaleDamage(reinterpret_cast<C_BasePlayer*>(fire_bullet_data.m_enter_trace.hit_entity), weapon_info, specific_hitgroup, fire_bullet_data.m_current_damage);
			}
			else
			{
				ScaleDamage(reinterpret_cast<C_BasePlayer*>(fire_bullet_data.m_enter_trace.hit_entity), weapon_info, fire_bullet_data.m_enter_trace.hitgroup, fire_bullet_data.m_current_damage);
			}

			return_info.iDamage = fire_bullet_data.m_current_damage;
			return_info.iHitgroup = fire_bullet_data.m_enter_trace.hitgroup;
			return_info.vecEnd = fire_bullet_data.m_enter_trace.endpos;
			return_info.HitEntity = (C_BasePlayer*)fire_bullet_data.m_enter_trace.hit_entity;

			break;
		}

		if (!HandleBulletPenetration(weapon_info, fire_bullet_data))
			break;

		return_info.bDidPenetrateWall = true;
	}

	scanned_damage.push_back(return_info.iDamage);
	scanned_points.push_back(pos);

	return_info.iPenetrationCount = fire_bullet_data.m_penetration_count;

	return return_info;

}

float AutoWall::HitgroupDamage (int iHitGroup) {
	switch (iHitGroup)
	{
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}

	return 1.0f;
}

void AutoWall::ScaleDamage (C_BasePlayer* e, CCSWeaponInfo* weapon_info, int hitgroup, float& current_damage) {
	static auto mp_damage_scale_ct_head = g_CVar->FindVar("mp_damage_scale_ct_head");
	static auto mp_damage_scale_t_head = g_CVar->FindVar("mp_damage_scale_t_head");
	static auto mp_damage_scale_ct_body = g_CVar->FindVar("mp_damage_scale_ct_body");
	static auto mp_damage_scale_t_body = g_CVar->FindVar("mp_damage_scale_t_body");

	auto team = e->m_iTeamNum();
	auto head_scale = team == 2 ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto body_scale = team == 2 ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto armor_heavy = e->m_bHasHeavyArmor();
	auto armor_value = static_cast<float>(e->m_ArmorValue());

	if (armor_heavy) head_scale *= 0.5f;

	// ref: CCSPlayer::TraceAttack

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage = (current_damage * 4.f) * head_scale;
		break;
	case HITGROUP_CHEST:
	case 8:
		current_damage *= body_scale;
		break;
	case HITGROUP_STOMACH:
		current_damage = (current_damage * 1.25f) * body_scale;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		current_damage *= body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage = (current_damage * 0.75f) * body_scale;
		break;
	default:
		break;
	}

	static auto IsArmored = [](C_BasePlayer* player, int hitgroup)
	{
		auto has_helmet = player->m_bHasHelmet();
		auto armor_value = static_cast<float>(player->m_ArmorValue());

		if (armor_value > 0.f)
		{
			switch (hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case 8:
				return true;
				break;
			case HITGROUP_HEAD:
				return has_helmet || (bool)player->m_bHasHeavyArmor();
				break;
			default:
				return (bool)player->m_bHasHeavyArmor();
				break;
			}
		}

		return false;
	};

	if (IsArmored (e, hitgroup))
	{
		auto armor_scale = 1.f;
		auto armor_ratio = (weapon_info->flArmorRatio * 0.5f);
		auto armor_bonus_ratio = 0.5f;

		if (armor_heavy)
		{
			armor_ratio *= 0.2f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.25f;
		}

		float new_damage = current_damage * armor_ratio;
		float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
		if (estiminated_damage > armor_value) new_damage = (current_damage - (armor_value / armor_bonus_ratio));

		current_damage = new_damage;
	}
}

bool AutoWall::HandleBulletPenetration (CCSWeaponInfo* info, FireBulletData_t& data, bool extracheck, Vector point) {
	CGameTrace trace_exit;
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.m_enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;

	static auto dmg_reduction_bullets =  g_CVar->FindVar("ff_damage_reduction_bullets")->GetFloat ();
	static auto dmg_bullet_penetration = g_CVar->FindVar("ff_damage_bullet_penetration")->GetFloat();

	float enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.18f;
	float compined_penetration_modifier = 0.f;
	bool solid_surf = ((data.m_enter_trace.contents >> 3) & CONTENTS_SOLID);
	bool light_surf = ((data.m_enter_trace.surface.flags >> 7) & SURF_LIGHT);

	if
		(
			data.m_penetration_count <= 0
			|| (!data.m_penetration_count && !light_surf && !solid_surf && enter_material != CHAR_TEX_GLASS && enter_material != CHAR_TEX_GRATE)
			|| info->flPenetration <= 0.f
			|| !TraceToExit(&data.m_enter_trace, data.m_enter_trace.endpos, data.m_direction, &trace_exit)
			&& !(g_EngineTrace->GetPointContents(data.m_enter_trace.endpos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX))
			)
	{
		return false;
	}

	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		compined_penetration_modifier = 3.f;
		final_damage_modifier = 0.05f;
	}
	else if (light_surf || solid_surf)
	{
		compined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else if (enter_material == CHAR_TEX_FLESH && data.m_enter_trace.hit_entity->m_iTeamNum() != g_LocalPlayer->m_iTeamNum() && !dmg_reduction_bullets) {

		if (!dmg_bullet_penetration)
			return false;

		compined_penetration_modifier = dmg_bullet_penetration;
		final_damage_modifier = 0.16f;
	}
	else {
		compined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			compined_penetration_modifier = 3.f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			compined_penetration_modifier = 2.0f;
	}

	float thickness = (trace_exit.endpos - data.m_enter_trace.endpos).LengthSqr();
	float modifier = std::max(0.f, 1.f / compined_penetration_modifier);

	if (extracheck) {
		static auto VectortoVectorVisible = [&](Vector src, Vector point) -> bool {
			trace_t TraceInit;
			TraceLine(src, point, MASK_SOLID, g_LocalPlayer, &TraceInit);
			trace_t Trace;
			TraceLine(src, point, MASK_SOLID, (C_BasePlayer*)TraceInit.hit_entity, &Trace);

			if (Trace.fraction == 1.0f || TraceInit.fraction == 1.0f)
				return true;

			return false;
		};
		if (!VectortoVectorVisible(trace_exit.endpos, point))
			return false;
	}

	float lost_damage = std::max(((modifier * thickness) / 24.f) + ((data.m_current_damage * final_damage_modifier) + (std::max(3.75f / info->flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > data.m_current_damage)
		return false;

	if (lost_damage > 0.f)
		data.m_current_damage -= lost_damage;

	if (data.m_current_damage < 1.f)
		return false;

	data.m_current_position = trace_exit.endpos;
	data.m_penetration_count--;

	return true;
}

bool AutoWall::TraceToExit (trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace) {
	Vector end;
	float distance = 0.f;
	signed int distance_check = 25;
	int first_contents = 0;

	do
	{
		distance += 3.5f;
		end = start + dir * distance;

		if (!first_contents) first_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		int point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(point_contents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || point_contents & CONTENTS_HITBOX && point_contents != first_contents)
		{
			Vector new_end = end - (dir * 4.f);

			Ray_t ray;
			ray.Init(end, new_end);

			g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
			{
				TraceLine(end, start, MASK_SHOT_HULL | CONTENTS_HITBOX, (C_BasePlayer*)exit_trace->hit_entity, exit_trace);

				if (exit_trace->DidHit() && !exit_trace->startsolid) return true;

				continue;
			}

			if (exit_trace->DidHit() && !exit_trace->startsolid)
			{
				if (enter_trace->surface.flags & SURF_NODRAW || !(exit_trace->surface.flags & SURF_NODRAW)) {
					if (exit_trace->plane.normal.Dot(dir) <= 1.f)
						return true;

					continue;
				}

				if (IsBreakableEntity((C_BasePlayer*)enter_trace->hit_entity)
					&& IsBreakableEntity((C_BasePlayer*)exit_trace->hit_entity))
					return true;

				continue;
			}

			if (exit_trace->surface.flags & SURF_NODRAW)
			{
				if (IsBreakableEntity((C_BasePlayer*)enter_trace->hit_entity)
					&& IsBreakableEntity((C_BasePlayer*)exit_trace->hit_entity))
				{
					return true;
				}
				else if (!(enter_trace->surface.flags & SURF_NODRAW))
				{
					continue;
				}
			}

			if ((!enter_trace->hit_entity
				|| enter_trace->hit_entity->EntIndex() == 0)
				&& (IsBreakableEntity((C_BasePlayer*)enter_trace->hit_entity)))
			{
				exit_trace = enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}


void AutoWall::TraceLine(Vector& start, Vector& end, unsigned int mask, C_BasePlayer* ignore, trace_t* trace) {
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	g_EngineTrace->TraceRay(ray, mask, &filter, trace);
}

void AutoWall::ClipTrace(Vector& start, Vector& end, C_BasePlayer* e, unsigned int mask, ITraceFilter* filter, trace_t* old_trace) {
	if (!e)
		return;

	Vector mins = e->GetCollideable()->OBBMins(), maxs = e->GetCollideable()->OBBMaxs();

	Vector dir(end - start);
	dir.NormalizeInPlace();

	Vector center = (maxs + mins) / 2, pos(center + e->m_vecOrigin());

	Vector to = pos - start;
	float range_along = dir.Dot(to);

	float range;

	if (range_along < 0.f)
	{
		range = -to.Length();
	}
	else if (range_along > dir.Length())
	{
		range = -(pos - end).Length();
	}
	else
	{
		auto ray(pos - (dir * range_along + start));
		range = ray.Length();
	}

	if (range <= 60.f) //55.f 
	{
		trace_t trace;

		Ray_t ray;
		ray.Init(start, end);

		g_EngineTrace->ClipRayToEntity(ray, mask, e, &trace);

		if (old_trace->fraction > trace.fraction) *old_trace = trace;
	}
}

bool AutoWall::IsBreakableEntity(C_BasePlayer* e) {
	if (!e || !e->EntIndex())
		return false;

	static auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(C_BasePlayer*)>(
		Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 51 56 8B F1 85 F6 74 68"));

	const auto result = is_breakable_fn(e);
	auto class_id = e->GetClientClass()->m_ClassID;
	if (!result && (class_id == 10 || class_id == 31 || (class_id == 11 && e->GetCollideable()->GetSolid() == 1)))
		return true;

	return result;
}

bool AutoWall::CanHitFloatingPoint(const Vector& point, const Vector& source) {

	static auto VectortoVectorVisible = [&](Vector src, Vector point) -> bool {
		trace_t TraceInit;
		TraceLine(src, point, MASK_SOLID, g_LocalPlayer, &TraceInit);
		trace_t Trace;
		TraceLine(src, point, MASK_SOLID, (C_BasePlayer*)TraceInit.hit_entity, &Trace);

		if (Trace.fraction == 1.0f || TraceInit.fraction == 1.0f)
			return true;

		return false;
	};

	FireBulletData_t data;
	data.m_start = source;
	data.m_filter = new CTraceFilter();
	data.m_filter->pSkip = g_LocalPlayer;
	Vector angles = Math::CalcAngle2(data.m_start, point);
	Math::AngleVectorS(angles, &data.m_direction);
	Math::VectorNormalize(data.m_direction);

	data.m_penetration_count = 1;
	auto weaponData = g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData();

	if (!weaponData)
		return false;

	data.m_current_damage = (float)weaponData->iDamage;
	Vector end = data.m_start + (data.m_direction * weaponData->flRange);
	TraceLine(data.m_start, end, MASK_SHOT | CONTENTS_HITBOX, g_LocalPlayer, &data.m_enter_trace);

	if (VectortoVectorVisible(data.m_start, point) || HandleBulletPenetration(weaponData, data, true, point))
		return true;

	delete data.m_filter;
	return false;
}