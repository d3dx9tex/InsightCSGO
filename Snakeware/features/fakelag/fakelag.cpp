#include "fakelag.h"
#include "../../options.hpp"

void Fakelag::Instance (CUserCmd * pCmd) {

	if (!g_Options.misc_fakelag)                     return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	if (g_EngineClient->IsVoiceRecording())          return;

	if (g_Options.misc_fakelag_on_shot && (pCmd->buttons & IN_ATTACK)) return;

	

	switch (g_Options.misc_fakelag_type) {

	case 0 :
		iChoke = std::min < int > (g_Options.misc_fakelag_ticks, 14);
		break;

	}

	Snakeware::bSendPacket = (iChoked > iChoke);

	if (Snakeware::bSendPacket)
		iChoked = 0;
	else
		iChoked++;

}
