#include "ragebot.h"
#include "../../options.hpp"
#include "autowall/ragebot-autowall.h"
#include "resolver/resolver.h"
#include <random>


int              iCurGroup;
static const int iTotalSeeds = 255;
static std::vector<std::tuple<float, float, float>> PreComputedSeeds = {};

void UpdateConfig () {
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return;


	if      (weapon->IsPistol() && (weapon->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)) iCurGroup = WEAPON_GROUPS::PISTOLS ;
	else if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)iCurGroup = WEAPON_GROUPS::H_PISTOLS;
	else if (weapon->IsRifle() || weapon->IsMashineGun()) iCurGroup = WEAPON_GROUPS::RIFLES;
	else if (weapon->IsSMG()) iCurGroup = WEAPON_GROUPS::SMG;
	else if (weapon->IsShotgun()) iCurGroup = WEAPON_GROUPS::SHOTGUNS;
	else if (weapon->IsAuto()) iCurGroup = WEAPON_GROUPS::AUTO;
	else if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) iCurGroup = WEAPON_GROUPS::SCOUT;
	else if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)  iCurGroup = WEAPON_GROUPS::AWP;
	else    iCurGroup = WEAPON_GROUPS::UNKNOWN;
}
void BulidSeedTable() {

	if (!PreComputedSeeds.empty()) return;

	for (auto i = 0; i < iTotalSeeds; i++) {
		RandomSeed(i + 1);

		const auto pi_seed = Math::RandomFloat(0.f, M_PI * 2);

		PreComputedSeeds.emplace_back(Math::RandomFloat(0.f, 1.f), sin(pi_seed), cos(pi_seed));
	}
}
void AutoRevolver(CUserCmd* cmd) {

	if (g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	if (cmd->buttons & IN_ATTACK)
		return;

	static auto r8cock_flag = true;
	static auto r8cock_time = 0.0f;

	r8cock_flag = true;

	if (r8cock_flag && g_LocalPlayer->m_hActiveWeapon()->CanFire() && !(g_LocalPlayer->m_hActiveWeapon()->m_flPostponeFireReadyTime() >= TICKS_TO_TIME(g_LocalPlayer->m_nTickBase())))
	{
		if (r8cock_time <= TICKS_TO_TIME(g_LocalPlayer->m_nTickBase()))
		{
			if (g_LocalPlayer->m_hActiveWeapon()->m_flNextSecondaryAttack() <= TICKS_TO_TIME(g_LocalPlayer->m_nTickBase()))
				r8cock_time = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase()) + 0.234375f;
			else
				cmd->buttons |= IN_ATTACK2;
		}
		else
			cmd->buttons |= IN_ATTACK;

		r8cock_flag = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase()) > r8cock_time;
	}
	else
	{
		r8cock_flag = false;
		r8cock_time = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase()) + 0.234375f;
		cmd->buttons &= ~IN_ATTACK;
	}
}
bool RageBot::IsValid (C_BasePlayer * Player) {

	if (!Player || Player == nullptr)						  return false;

	if (Player->IsLocalPlayer())						      return false;

	if (Player->IsTeam())									  return false;

	if (Player->IsDormant())								  return false;

	if (Player->m_bGunGameImmunity())						  return false;

	if (!Player->IsAlive())									  return false;

	if (Player->GetPlayerInfo().szName == "ba1m0v")           return false;

	return true;
}

void RageBot::Instance (CUserCmd * Cmd) {

	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !g_Options.ragebot_enabled)  return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())                                                  return;

	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	pCmd         = Cmd;

	if (!pWeapon || pWeapon->IsKnife() || pWeapon->IsGrenade() || pWeapon->IsWeaponNonAim())          return;

	UpdateConfig();
	AutoRevolver(pCmd);

	bool bFindNewTarget = true; // Scan a new target..
	int  iBestFov       = 180;
	int  iBestHealth    = 101;

	
	LagCompensation::Get().StartPositionAdjustment();
	if (bFindNewTarget) {

		
		iTargetID = 0;
		pTarget   = nullptr;

		switch (g_Options.ragebot_selection) {
		case 0: iTargetID = GetTargetFOV     ();
		case 1: iTargetID = GetTargetDistance();
		case 2: iTargetID = GetTargetHealth  ();
		}

		if (iTargetID >= 0) {
			pTarget = static_cast< C_BasePlayer *> ( g_EntityList->GetClientEntity(iTargetID));
		}
		else {
			pTarget = nullptr;
		}
		
	}


	

	if (iTargetID >= 0 && pTarget) {
		auto pAimpoint = Scan();

		if (!pAimpoint.Empty()) {

			Aim(pAimpoint, iTargetID);
		}
	}

	LagCompensation::Get().FinishPositionAdjustment();

	if (!IsValid(pTarget)) {
		LagCompensation::Get().Reset();
	}

	if (pCmd->buttons & IN_ATTACK) {
		if (g_Options.ragebot_remove_recoil)
			pCmd->viewangles -= g_LocalPlayer->m_aimPunchAngle() * 2.f;
	}
}

int RageBot::GetTargetFOV ( ) {
	
	int   iTarget = -1;
	float flFov = 180;
	QAngle vecView;        g_EngineClient->GetViewAngles(&vecView);

	auto BoundingBoxCheck = [this] (C_BasePlayer* entity) -> bool {

		auto collideable = entity->GetCollisionState();

		if (!collideable)
			return false;

		const auto bbmin = collideable->OBBMins() + entity->m_vecOrigin();
		const auto bbmax = collideable->OBBMaxs() + entity->m_vecOrigin();
		Vector Points[7];

		Points[0] = entity->GetHitboxPos(0);
		Points[1] = (bbmin + bbmax) * 0.5f;
		Points[2] = Vector((bbmax.x + bbmin.x) * 0.5f, (bbmax.y + bbmin.y) * 0.5f, bbmin.z);

		Points[3] = bbmax;
		Points[4] = Vector (bbmax.x, bbmin.y, bbmax.z);
		Points[5] = Vector (bbmin.x, bbmin.y, bbmax.z);
		Points[6] = Vector (bbmin.x, bbmax.y, bbmax.z);

		for (const auto& Point : Points) {

			if (AutoWall::Get().GetPointDamage(Point,entity) > 0)
				return true;
		}
		return false;
	};

	for (int i = 0; i < 64; i++) {

		C_BasePlayer * pSnake = static_cast <C_BasePlayer *> (g_EntityList->GetClientEntity(i));

		if (IsValid (pSnake)) {
			if (BoundingBoxCheck(pSnake)) {

				float FoV = Math::GetFOV(vecView, Math::CalcAngle(pSnake->m_vecOrigin() , g_LocalPlayer->m_vecOrigin()));

				if (FoV < flFov && FoV < g_Options.ragebot_fov) {
					flFov = FoV;
					iTarget = i;
				}
			}
		}
	}
	return iTarget;
}

int RageBot::GetTargetDistance ( ) {
	
	int iTarget = -1;
	int iMinDist = 99999;

	QAngle vecView; g_EngineClient->GetViewAngles(&vecView);
	auto BoundingBoxCheck = [this](C_BasePlayer* entity) -> bool {

		auto collideable = entity->GetCollisionState();

		if (!collideable)
			return false;

		const auto bbmin = collideable->OBBMins() + entity->m_vecOrigin();
		const auto bbmax = collideable->OBBMaxs() + entity->m_vecOrigin();
		Vector Points[7];

		Points[0] = entity->GetHitboxPos(0);
		Points[1] = (bbmin + bbmax) * 0.5f;
		Points[2] = Vector((bbmax.x + bbmin.x) * 0.5f, (bbmax.y + bbmin.y) * 0.5f, bbmin.z);

		Points[3] = bbmax;
		Points[4] = Vector(bbmax.x, bbmin.y, bbmax.z);
		Points[5] = Vector(bbmin.x, bbmin.y, bbmax.z);
		Points[6] = Vector(bbmin.x, bbmax.y, bbmax.z);

		for (const auto& Point : Points) {

			if (AutoWall::Get().GetPointDamage(Point, entity) > 0)
				return true;
		}
		return false;
	};

	for (int i = 0; i < 64; i++) {
		C_BasePlayer * pSnake = static_cast <C_BasePlayer *> (g_EntityList->GetClientEntity(i));
		if (IsValid (pSnake)) {
			if (BoundingBoxCheck(pSnake)) {
				Vector Difference = g_LocalPlayer->m_vecOrigin() - pSnake->m_vecOrigin();
				int Distance = Difference.Length();
				float FoV = Math::GetFOV(vecView, Math::CalcAngle(pSnake->m_vecOrigin(), g_LocalPlayer->m_vecOrigin()));

				if (Distance < iMinDist  && FoV < g_Options.ragebot_fov)	{
					iMinDist = Distance;
					iTarget = i;
				}
			}
		}
	}
	return iTarget;
}

int RageBot::GetTargetHealth ( ) {
	
	int iTarget    = -1;
	int iMinHealth = 101;	
	QAngle View; g_EngineClient->GetViewAngles(&View);

	auto BoundingBoxCheck = [this](C_BasePlayer* entity) -> bool {

		auto collideable = entity->GetCollisionState();

		if (!collideable)
			return false;

		const auto bbmin = collideable->OBBMins() + entity->m_vecOrigin();
		const auto bbmax = collideable->OBBMaxs() + entity->m_vecOrigin();
		Vector Points[7];

		Points[0] = entity->GetHitboxPos(0);
		Points[1] = (bbmin + bbmax) * 0.5f;
		Points[2] = Vector((bbmax.x + bbmin.x) * 0.5f, (bbmax.y + bbmin.y) * 0.5f, bbmin.z);

		Points[3] = bbmax;
		Points[4] = Vector(bbmax.x, bbmin.y, bbmax.z);
		Points[5] = Vector(bbmin.x, bbmin.y, bbmax.z);
		Points[6] = Vector(bbmin.x, bbmax.y, bbmax.z);

		for (const auto& Point : Points) {

			if (AutoWall::Get().GetPointDamage( Point, entity) > 0)
				return true;
		}
		return false;
	};


	for (int i = 0; i < 64; i++) {
		C_BasePlayer * pSnake = static_cast <C_BasePlayer *> (g_EntityList->GetClientEntity(i));
		if (IsValid(pSnake)) {
			if (BoundingBoxCheck(pSnake)) {
				int Health = pSnake->m_iHealth();
				float FoV = Math::GetFOV(View, Math::CalcAngle(pSnake->m_vecOrigin(), g_LocalPlayer->m_vecOrigin()));
				if (Health < iMinHealth && FoV < g_Options.ragebot_fov)
				{
					iMinHealth = Health;
					iTarget = i;
				}
			}
		}
	}
	return iTarget;
}

void RageBot::ResetTarget() {

	Snakeware::OnShot = false;
	iTargetID         = -1;
	
}

bool RageBot::Hitchance (QAngle Aimangle) {

	BulidSeedTable();

	float flChance;
	auto  wep = g_LocalPlayer->m_hActiveWeapon();
	auto  wepidx = wep->m_Item().m_iItemDefinitionIndex();


	if (!wep)
		return false;



	if (wepidx == WEAPON_ZEUS)
		flChance = 80.f;
	else 
		flChance = g_Options.ragebot_hitchance[iCurGroup];


	Vector fw, rw, uw;
	Math::AngleVectors(Aimangle, fw, rw, uw);

	int hits = 0;
	int needed_hits = static_cast<int>(iTotalSeeds * (flChance / 100.f));

	
	float cone = wep->GetSpread();
	float inacc = wep->GetInaccuracy();

	Vector src = g_LocalPlayer->GetEyePos();

	for (int i = 0; i < iTotalSeeds; i++) {

		//fSeed = &PreComputedSeeds[i];

		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, M_PI * 2.f);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, M_PI * 2.f);
		float inaccuracy = a * inacc;
		float spread     = c * cone;

		if (wepidx == WEAPON_REVOLVER) {
			if (pCmd->buttons & IN_ATTACK2) {
				a = 1.f - a * a;
				c = 1.f - c * c;
			}
		}

		Vector spread_view((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0);
		Vector direction;

		direction.x = fw.x + (spread_view.x * rw.x) + (spread_view.y * uw.x);
		direction.y = fw.y + (spread_view.x * rw.y) + (spread_view.y * uw.y);
		direction.z = fw.z + (spread_view.x * rw.z) + (spread_view.y * uw.z);
		direction.Normalized();

		Vector viewangles_spread;
		Vector view_forward;

		Math::Vector_Angles(direction, uw, viewangles_spread);
		viewangles_spread.Normalize();
		Math::AngleVectors2(viewangles_spread, view_forward);

		view_forward.NormalizeInPlace();
		view_forward = src + (view_forward * wep->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init (src, view_forward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, pTarget, &tr);

		if (tr.hit_entity == pTarget)
			hits++;

		if (static_cast<int>((static_cast<float>(hits) / iTotalSeeds) * 100.f) >= flChance)
			return true;

		if ((iTotalSeeds - i + hits) < needed_hits)
			return false;
	}

	return false;
}


void RageBot::Multipoints (int hitbox, matrix3x4_t bones[128], std::vector<Vector>& points) {

	if (!g_Options.ragebot_multipoint)
		return;

	
	auto mdl = g_MdlInfo->GetStudioModel(pTarget->GetModel());
	auto set = mdl->GetHitboxSet        (pTarget->m_nHitboxSet());
	auto hbx = set->GetHitbox(hitbox);

	if (!hbx)
		return;

	Vector mins, maxs;
	Math::VectorTransform(hbx->bbmin, bones[hbx->bone], mins);
	Math::VectorTransform(hbx->bbmax, bones[hbx->bone], maxs);

	Vector center = (mins + maxs) * 0.5f;
	Vector angle  = Math::CalcAngle2(center, g_LocalPlayer->GetEyePos());

	Vector forward;
	Math::AngleVectors2(angle, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);
	Vector top = Vector(0, 0, 1);
	Vector bottom = Vector(0, 0, -1);

	float adjusted_radius = 0.f;
	switch (hitbox) {
	case HITBOX_HEAD:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_pointscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + top * adjusted_radius });
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
		}
		break;

	case HITBOX_NECK:
		adjusted_radius = hbx->m_flRadius * float(0.f / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
		}
		break;

	case HITBOX_CHEST:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_bodyscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
			points.push_back({ center + bottom * adjusted_radius });
		}
		break;

	case HITBOX_UPPER_CHEST:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_bodyscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + top * adjusted_radius });
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
		}
		break;

	case HITBOX_STOMACH:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_bodyscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + top * adjusted_radius });
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
		}
		break;

	case HITBOX_PELVIS:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_bodyscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + right * adjusted_radius });
			points.push_back({ center + left * adjusted_radius });
			points.push_back({ center + bottom * adjusted_radius });
		}
		break;

	case HITBOX_LEFT_FOREARM:
	case HITBOX_RIGHT_FOREARM:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_otherscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + top * adjusted_radius });
			points.push_back({ center + bottom * adjusted_radius });
		}
		break;

	case HITBOX_LEFT_CALF:
	case HITBOX_RIGHT_CALF:
		adjusted_radius = hbx->m_flRadius * float(g_Options.ragebot_otherscale[iCurGroup] / 100.f);

		if (adjusted_radius > 0.f) {
			points.push_back({ center + top * adjusted_radius });
			points.push_back({ center + bottom * adjusted_radius });
		}
		break;
	}
}

bool RageBot::IsAbleToShoot() {
	auto wep = g_LocalPlayer->m_hActiveWeapon();
	if (!wep)                                                 return false;

	if (wep->m_Item() .m_iItemDefinitionIndex() == WEAPON_C4) return true;

	if (wep->IsWeaponNonAim())                                return false;

	auto time = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase());

	if (pCmd->weaponselect)
		return false;

	if (wep->m_iClip1() < 1)
		return false;

	if ((g_LocalPlayer->m_flNextAttack() > time) || wep->m_flNextPrimaryAttack() > time || wep->m_flNextSecondaryAttack() > time)
	{
		if (wep->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			return true;
		else
		return false;
	}

	return true;
}

bool RageBot::CanHitHitbox(Vector vecStart, Vector vecEnd, C_BasePlayer* pPlayer, int box, bool in_shot, BoneArray* bones) {
	if (!pPlayer || !pPlayer->IsAlive())
		return false;

	// always try to use our aimbot matrix first.
	auto matrix = bones;

	// this is basically for using a custom matrix.
	if (in_shot)
		matrix = bones;

	if (!matrix)	
		return false;

	const model_t* model = pPlayer->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_MdlInfo->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(pPlayer->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;

	Vector min, max;
	const auto IsCapsule = bbox->m_flRadius != -1.f;

	if (IsCapsule) {
		Math::VectorTransform(bbox->bbmin, matrix[bbox->bone], min);
		Math::VectorTransform(bbox->bbmax, matrix[bbox->bone], max);
		const auto dist = Math::Segment2Segment(vecStart, vecEnd, min, max);

		if (dist < bbox->m_flRadius) {
			return true;
		}
	}
	else {

		Math::VectorTransform(Math::VectorRotate(bbox->bbmin, bbox->rotation), matrix[bbox->bone], min);
		Math::VectorTransform(Math::VectorRotate(bbox->bbmax, bbox->rotation), matrix[bbox->bone], max);

		Math::vector_i_transform(vecStart, matrix[bbox->bone], min);
		Math::vector_i_transform(vecEnd, matrix[bbox->bone], max);

		if (Math::intersect_line_with_bb(min, max, bbox->bbmin, bbox->bbmax))
			return true;
	}

	return false;
}

bool RageBot::CheckSafePoint(Vector  pPoint) {

	if (pPoint == Vector (0, 0, 0)) return false;

	for (int iSafeBox = HITBOX_HEAD; iSafeBox < HITBOX_LEFT_THIGH; iSafeBox++) {

		if (CanHitHitbox(g_LocalPlayer->GetShootPos(), pPoint, pTarget, iSafeBox, true, Resolver::Get().pMiddleMatrix) &&
			CanHitHitbox(g_LocalPlayer->GetShootPos(), pPoint, pTarget, iSafeBox, true, Resolver::Get().pLeftMatrix) &&
			CanHitHitbox(g_LocalPlayer->GetShootPos(), pPoint, pTarget, iSafeBox, true, Resolver::Get().pRightMatrix))
			return true;
	}

	return false;
}

Vector CurentAimVector;
C_BasePlayer* CurentTarget;

Vector RageBot::Scan(int  *iHitbox ,int* estimated_damage) {

	std::vector<Vector> points = {};

	
	auto wep     = g_LocalPlayer->m_hActiveWeapon();
	auto wepdmg  = wep->GetCSWeaponData()->iDamage;
	auto wepidx  = wep->m_Item().m_iItemDefinitionIndex();
	auto enemyhp = pTarget->m_iHealth();
	auto ent     = pTarget->EntIndex();

	auto force_baim = false;

	if (g_Options.ragebot_baim_if_lethal) {
		auto aw_dmg1 = AutoWall::Get().GetPointDamage(pTarget->GetHitboxPos(HITBOX_STOMACH),  pTarget);
		auto aw_dmg2 = AutoWall::Get().GetPointDamage(pTarget->GetHitboxPos(HITBOX_PELVIS),   pTarget);
		auto aw_dmg3 = AutoWall::Get().GetPointDamage(pTarget->GetHitboxPos(HITBOX_CHEST),    pTarget);
		if ((aw_dmg1 > enemyhp) || (aw_dmg2 > enemyhp) || (aw_dmg3 > enemyhp)) {
			force_baim = true;
		}
	}
	
	if (GetAsyncKeyState(g_Options.ragebot_baim_key))
		force_baim = true;

	if (wepidx == WEAPON_ZEUS || wep->IsKnife())
		force_baim = true;

	if (g_Options.ragebot_adaptive_baim && (pTarget->m_iHealth() <= 39 || !(pTarget->m_fFlags() & FL_ONGROUND || (pTarget->m_vecVelocity().Length2D() >= 202))))
		force_baim = true;

	
	pTarget->ForceBoneRebuilid ();
	pTarget->SetupBonesFixed(bones, BONE_USED_BY_ANYTHING); // rebulid time

	if (bones == nullptr)
		return Vector(0, 0 ,0);
	

	if (g_Options.ragebot_hitbox[0][iCurGroup] && !force_baim) {

		points.push_back({ pTarget->GetHitboxPos(HITBOX_HEAD) });

		if (g_Options.ragebot_multipoint ) {
			Multipoints(HITBOX_HEAD, bones, points);
		}
	}

	if (g_Options.ragebot_hitbox[1][iCurGroup] && !force_baim) {
		points.push_back({ pTarget->GetHitboxPos(HITBOX_NECK) });
		if (g_Options.ragebot_multipoint) {
			Multipoints(HITBOX_NECK, bones, points);
		}
	}

	if (g_Options.ragebot_hitbox[2][iCurGroup] && !force_baim) {
		points.push_back({ pTarget->GetHitboxPos(HITBOX_CHEST) });
		points.push_back({ pTarget->GetHitboxPos(HITBOX_LOWER_CHEST) });
		points.push_back({ pTarget->GetHitboxPos(HITBOX_UPPER_CHEST) });
		if (g_Options.ragebot_multipoint && !(GetAsyncKeyState(g_Options.ragebot_force_safepoint))) {
			Multipoints(HITBOX_CHEST, bones, points);
			Multipoints(HITBOX_UPPER_CHEST, bones, points);
		}
	}

	if (g_Options.ragebot_hitbox[3][iCurGroup] && force_baim) {
		points.push_back({ pTarget->GetHitboxPos(HITBOX_STOMACH) });
		points.push_back({ pTarget->GetHitboxPos(HITBOX_PELVIS) });
		if (g_Options.ragebot_multipoint && !(GetAsyncKeyState(g_Options.ragebot_force_safepoint))) {
			Multipoints           (HITBOX_STOMACH, bones, points);
			Multipoints           (HITBOX_PELVIS, bones, points);
		}
	}
	if (pTarget->m_vecVelocity().Length2D() <= 0.15f) {

		if (g_Options.ragebot_hitbox[4][iCurGroup] && !force_baim) {
			points.push_back({ pTarget->GetHitboxPos(HITBOX_LEFT_FOREARM) });
			points.push_back({ pTarget->GetHitboxPos(HITBOX_RIGHT_FOREARM) });
			if (g_Options.ragebot_multipoint) {
				Multipoints(HITBOX_LEFT_FOREARM, bones, points);
				Multipoints(HITBOX_RIGHT_FOREARM, bones, points);
			}
		}

		if (g_Options.ragebot_hitbox[5][iCurGroup] && !force_baim) {
			points.push_back({ pTarget->GetHitboxPos(HITBOX_LEFT_CALF) });
			points.push_back({ pTarget->GetHitboxPos(HITBOX_RIGHT_CALF) });
			points.push_back({ pTarget->GetHitboxPos(HITBOX_LEFT_THIGH) });
			points.push_back({ pTarget->GetHitboxPos(HITBOX_RIGHT_THIGH) });
			if (g_Options.ragebot_multipoint) {
				Multipoints(HITBOX_LEFT_CALF, bones, points);
				Multipoints(HITBOX_RIGHT_CALF, bones, points);
				Multipoints(HITBOX_LEFT_THIGH, bones, points);
				Multipoints(HITBOX_RIGHT_THIGH, bones, points);
			}
		}
	}

	auto BackupMins  = pTarget->GetCollideable()->OBBMins();
	auto BackupMaxs  = pTarget->GetCollideable()->OBBMaxs();

	auto BackupCache      = *pTarget->CachedBones();
	auto BackupCacheCount = pTarget->CachedBonesCount();

	pTarget->CachedBonesCount()  = 128;
	*pTarget->CachedBones()      = bones;

	int best_damage = 0;
	Vector best_point = Vector (0, 0, 0);

	for (auto point : points) {

		auto  WallDamage = AutoWall::Get().GetPointDamage(point, pTarget);

		if   (WallDamage <= 0) continue;

		CurentAimVector = point;

		bool bIsSafePoint = CheckSafePoint (point);
			
		if (GetAsyncKeyState(g_Options.ragebot_force_safepoint) && !bIsSafePoint) {

			bSafePointBox = true;

			points.emplace_back(point);

			continue;

		}

		auto isVisible = g_LocalPlayer->PointVisible(point);
		if (g_Options.ragebot_autowall && !isVisible) {
				
				if (wep) {
					if (WallDamage > best_damage && WallDamage >= g_Options.ragebot_mindamage[iCurGroup]) {

						best_damage = WallDamage;
						best_point = point;

					}
					else if (WallDamage > best_damage && WallDamage) {
						if (WallDamage > 25) {
							PredictiveAStop( (WallDamage / pTarget->m_iHealth()) );
						}
					}
				}
			
		}

		if (isVisible) { 

				if (wep) {
					if (WallDamage > best_damage && WallDamage >= g_Options.ragebot_vis_mindamage[iCurGroup]) {

						best_damage = WallDamage;
						best_point  = point;
					
					}
				}
		}

	}
	pTarget->GetCollideable()->OBBMins() = BackupMins;
	pTarget->GetCollideable()->OBBMaxs() = BackupMaxs;

	*pTarget->CachedBones()      = BackupCache;
	 pTarget->CachedBonesCount() = BackupCacheCount;

	if (estimated_damage)
		*estimated_damage = best_damage;

	return best_point;
}


Vector RageBot::GetHitboxPosition (C_BasePlayer* pPlayer, int iHitbox, matrix3x4_t Matrix[]) {
	if (iHitbox < 0 || iHitbox > 19) return Vector (0, 0, 0);

	if (!pPlayer) return Vector(0, 0, 0);

	

	if (!pPlayer->GetClientRenderable())
		return Vector(0, 0, 0);

	const auto model = pPlayer->GetModel();

	if (!model)
		return Vector(0, 0, 0);

	auto pStudioHdr = g_MdlInfo->GetStudioModel(model);

	if (!pStudioHdr)
		return Vector(0, 0, 0);

	auto hitbox = pStudioHdr->pHitbox(iHitbox, pPlayer->m_nHitboxSet());

	if (!hitbox)
		return Vector(0, 0, 0);

	if (hitbox->bone > 128 || hitbox->bone < 0)
		return Vector(0, 0, 0);

	Vector min, max;
	Math::VectorTransform(hitbox->bbmin, Matrix[hitbox->bone], min);
	Math::VectorTransform(hitbox->bbmax, Matrix[hitbox->bone], max);

	auto center = (min + max) / 2.f;

	return center;
}


bool RageBot::Aim(Vector point, int idx) {

	auto wep           =  g_LocalPlayer->m_hActiveWeapon();
	float flServerTime =  g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
	bool canShoot      = (wep->m_flNextPrimaryAttack() <= flServerTime && wep->m_iClip1() > 0);
	auto aimangle      = Math::CalcAngle (g_LocalPlayer->GetEyePos(), point);
	QAngle localview;
	g_EngineClient->GetViewAngles(&localview);
	auto shoot_state = this->IsAbleToShoot();
	bool Hitchanced  = this->Hitchance(aimangle);
	auto Idx = pTarget->EntIndex() - 1;

	auto TickRecord = -1;
	Snakeware::OnShot = false;

	auto IsValidTick = Snakeware::pLagRecords[Idx].TickCount != -1; // aye

	static int last_shot;
	static int last_pitch_up;

	if (pTarget->m_angEyeAngles().pitch < 60)
		last_pitch_up = g_GlobalVars->realtime;

	bool OnShot = true;


	if  (Hitchanced) {

		if (g_Options.ragebot_autofire && shoot_state) {

			if (OnShot) {
				Snakeware::OnShot = true;
				if (IsValidTick)
					pCmd->tick_count = Snakeware::pLagRecords[Idx].TickCount;
				pCmd->buttons |= IN_ATTACK;
			}
		}
		if (pCmd->buttons & IN_ATTACK) {

			Snakeware::bSendPacket = true;
			pCmd->viewangles       = aimangle;
			if (!g_Options.ragebot_silent) {
				g_EngineClient->SetViewAngles (&aimangle);
			}

		}
	}
	else {
		if (g_Options.ragebot_autoscope && wep->IsSniper() &&  !g_LocalPlayer->m_bIsScoped()) {
			pCmd->buttons |= IN_ATTACK2;
		}


	}
	if (g_Options.ragebot_autostop) {
		static int MinimumVelocity = 0;
		bool shouldstop = g_Options.ragebot_beetweenshots ? true : canShoot;
		MinimumVelocity = wep->GetCSWeaponData()->flMaxPlayerSpeedAlt * .34f;
		if (g_LocalPlayer->m_vecVelocity().Length() >= MinimumVelocity && shouldstop && !GetAsyncKeyState(VK_SPACE) && !wep->IsKnife() && wep->m_Item().m_iItemDefinitionIndex() != WEAPON_ZEUS)
			QuickStop();
	}
	return true;
}

void RageBot::QuickStop () {

	if (!g_Options.ragebot_autostop)
		return;

	/* Default autostop method */ {

		QAngle direction;
		QAngle real_view;

		Math::VectorAngles(g_LocalPlayer->m_vecVelocity(), direction);
		g_EngineClient->GetViewAngles(&real_view);

		direction.yaw = real_view.yaw - direction.yaw;

		Vector forward;
		Math::AngleVectors(direction, forward);

		static auto cl_forwardspeed = g_CVar->FindVar(Xor("cl_forwardspeed"));
		static auto cl_sidespeed = g_CVar->FindVar(Xor("cl_sidespeed"));

		auto negative_forward_speed = -cl_forwardspeed->GetFloat() * 1.3;
		auto negative_side_speed = -cl_sidespeed->GetFloat() * 1.3;

		auto negative_forward_direction = forward * negative_forward_speed;
		auto negative_side_direction = forward * negative_side_speed;

		pCmd->forwardmove = negative_forward_direction.x;
		pCmd->sidemove = negative_side_direction.y;
	}

}
void RageBot::PredictiveAStop(int value) {

	if (!g_Options.ragebot_autostop)
		return;

	/* predictive autostop method */ {

		QAngle direction;
		QAngle real_view;

		Math::VectorAngles(g_LocalPlayer->m_vecVelocity(), direction);
		g_EngineClient->GetViewAngles(&real_view);

		direction.yaw = real_view.yaw - direction.yaw;

		Vector forward;
		Math::AngleVectors(direction, forward);

		static auto cl_forwardspeed = g_CVar->FindVar(Xor("cl_forwardspeed"));
		static auto cl_sidespeed	= g_CVar->FindVar(Xor("cl_sidespeed"));

		auto negative_forward_speed = -cl_forwardspeed->GetFloat() * (value * 0.25);
		auto negative_side_speed	= -cl_sidespeed->GetFloat() * (value * 0.25);

		auto negative_side_direction = Vector(g_LocalPlayer->m_vecVelocity().x * 0.6, g_LocalPlayer->m_vecVelocity().y * 0.6, g_LocalPlayer->m_vecVelocity().z * 0.6);

		pCmd->forwardmove -= negative_side_direction.x;
		pCmd->sidemove -= negative_side_direction.y;
	}
}


