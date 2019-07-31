#pragma once

#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

class As_mx_transitions_easing_None : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_None(ActionContext* context);

private:
	void None_easeIn(CallArgs& ca);

	void None_easeInOut(CallArgs& ca);

	void None_easeOut(CallArgs& ca);

	void None_easeNone(CallArgs& ca);
};

	}
}

