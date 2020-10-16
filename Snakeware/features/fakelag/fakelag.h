#pragma once

#include "../../valve_sdk/csgostructs.hpp"


class Fakelag : public Singleton < Fakelag > {

   public:

	   void Instance (CUserCmd * pCmd);

	   int iChoke = 0;
	   int iChoked = 0;
};

