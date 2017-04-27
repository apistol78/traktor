/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_mx_transitions_easing_Regular.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Regular", As_mx_transitions_easing_Regular, ActionObject)

As_mx_transitions_easing_Regular::As_mx_transitions_easing_Regular(ActionContext* context)
:	ActionObject(context)
{
	setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Regular::Regular_easeIn)));
	setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Regular::Regular_easeInOut)));
	setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Regular::Regular_easeOut)));
}

void As_mx_transitions_easing_Regular::Regular_easeIn(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();

	T /= D;
	ca.ret = ActionValue(C * T * T * T + B);
}

void As_mx_transitions_easing_Regular::Regular_easeInOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();

	T /= D * 0.5f;
	if (T < 1.0f)
		ca.ret = ActionValue(C / 2.0f * T * T * T + B);
	else
	{
		T -= 2.0f;
		ca.ret = ActionValue(C / 2.0f * (T * T * T + 2.0f) + B);
	}
}

void As_mx_transitions_easing_Regular::Regular_easeOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();

	T = (T / D) - 1.0f;
	ca.ret = ActionValue(C * (T * T * T + 1.0f) + B);
}

	}
}
