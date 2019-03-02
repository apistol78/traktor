#pragma once

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Bounce : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Bounce(ActionContext* context);

private:
	void Bounce_easeIn(CallArgs& ca);

	void Bounce_easeInOut(CallArgs& ca);

	void Bounce_easeOut(CallArgs& ca);
};

	}
}

