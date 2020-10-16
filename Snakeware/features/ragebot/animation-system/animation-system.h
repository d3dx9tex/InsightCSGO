#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include <unordered_map>
#include <deque>
#include <optional>
#include <functional>


class Animations : public Singleton<Animations> {
public:
	
	float m_ServerAbsRotation = 0.f;

	void FakeAnimation            ();

	void FixLocalPlayer           ();

	void SetLocalPlayerAnimations ();




	// fake  matrix & state
	CCSGOPlayerAnimState* RealAnimstate = nullptr;
	CCSGOPlayerAnimState* FakeAnimstate = nullptr;




	QAngle m_current_real_angle = QAngle(0.f, 0.f, 0.f);
	QAngle m_current_fake_angle = QAngle(0.f, 0.f, 0.f);
	

private:

};