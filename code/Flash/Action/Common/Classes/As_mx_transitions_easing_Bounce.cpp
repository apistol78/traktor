/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_mx_transitions_easing_Bounce.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Bounce", As_mx_transitions_easing_Bounce, ActionObject)

As_mx_transitions_easing_Bounce::As_mx_transitions_easing_Bounce(ActionContext* context)
:	ActionObject(context)
{
	setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeIn)));
	setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeInOut)));
	setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeOut)));
}

void As_mx_transitions_easing_Bounce::Bounce_easeIn(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Bounce::Bounce_easeInOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Bounce::Bounce_easeOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
