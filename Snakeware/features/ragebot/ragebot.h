
#include "../../valve_sdk/csgostructs.hpp"
#include "../../helpers/math.hpp"
#include <vector>
// Vip snakeware ragebot...
// by @Snake


struct ScanData {
	// nano-tech by 5N4K3

};



class RageBot : public Singleton<RageBot> {
public:
	void           Instance     (CUserCmd * Cmd);
	int            GetTargetFOV ();

	int GetTargetDistance();

	int GetTargetHealth();

	void ResetTarget();

	bool Hitchance (QAngle Aimangle);

	void Multipoints(int hitbox, matrix3x4_t bones[128], std::vector<Vector>& points);

	bool IsAbleToShoot ();

	bool CanHitHitbox (Vector vecStart, Vector vecEnd, C_BasePlayer * pPlayer, int box, bool in_shot, BoneArray * bones);

     bool  CheckSafePoint (Vector  pPoint);


	Vector Scan (int *iHitbox = nullptr , int * estimated_damage = nullptr);

	Vector GetHitboxPosition (C_BasePlayer * pPlayer, int iHitbox, matrix3x4_t Matrix[]);

	bool   Aim               (Vector point, int idx);

	void QuickStop();



	bool IsValid (C_BasePlayer * Player);

	matrix3x4_t bones[128];

	C_BasePlayer*  pTarget = nullptr;
	int            iTargetID = 0;
	CUserCmd      *pCmd; 
	bool           bSafePointBox = false;
	

	Vector OldPoint[65];
};