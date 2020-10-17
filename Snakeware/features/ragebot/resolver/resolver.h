#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../lagcompensation/lag-compensation.h"
#include "../animation-system/animation-system.h"
class QAngle;
class C_BasePlayer;

struct ResolveInfo {
	ResolveInfo() {	 }

	AnimationLayer          ResolverLayers[3][13];
	int                     iResolvingWay;
	int                     iMissedShots = 0;
	bool                    bWasUpdated;
};

class Resolver : public Singleton < Resolver > {
public :

	void PreverseSafePoint (C_BasePlayer * pPlayer, int iSafeSide, float flTime);

	void DetectFakeSide    (C_BasePlayer * pPlayer);
	void StoreResolveDelta (C_BasePlayer * pPlayer, ResolveInfo * cData);


	ResolveInfo ResolveRecord [64];
	
	// SafePoint side's.
	BoneArray pLeftMatrix    [MAXSTUDIOBONES];
	BoneArray pMiddleMatrix  [MAXSTUDIOBONES];
	BoneArray pRightMatrix   [MAXSTUDIOBONES];
	BoneArray pDefaultMatrix [MAXSTUDIOBONES];
};