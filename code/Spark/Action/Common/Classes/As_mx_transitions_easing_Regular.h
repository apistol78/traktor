#pragma once

#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

class As_mx_transitions_easing_Regular : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Regular(ActionContext* context);

private:
	void Regular_easeIn(CallArgs& ca);

	void Regular_easeInOut(CallArgs& ca);

	void Regular_easeOut(CallArgs& ca);
};

	}
}

