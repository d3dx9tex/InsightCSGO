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
	int                     iResolvingWay		 ;
	int                     iMissedShots = 0	 ;
	bool                    bWasUpdated			 ;
	std::string			    info				 ;
	bool					has_fake		     ;
	bool					has_fakelags		 ;


	/*			  lby desync mode	     		*/	
	int						LastBalancedDesync	 ;
	int						lby_delta			 ;
	bool					desyncmode_balance	 ;

	int						LastPitchDown		 ;

};

class Resolver : public Singleton < Resolver > {
public :

	void PreverseSafePoint (C_BasePlayer * pPlayer, int iSafeSide, float flTime);

	void DetectFakeSide    (C_BasePlayer * pPlayer);
	void StoreStatusPlayer (C_BasePlayer* pPlayer, int resolve_info, int side, bool backward);
	void StoreResolveDelta (C_BasePlayer * pPlayer, ResolveInfo * cData);


	ResolveInfo ResolveRecord [65];
	
	// SafePoint side's.
	BoneArray pLeftMatrix    [MAXSTUDIOBONES];
	BoneArray pMiddleMatrix  [MAXSTUDIOBONES];
	BoneArray pRightMatrix   [MAXSTUDIOBONES];
	BoneArray pDefaultMatrix [MAXSTUDIOBONES];
};