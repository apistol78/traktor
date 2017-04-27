/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_mx_transitions_easing_Back.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Back", As_mx_transitions_easing_Back, ActionObject)

As_mx_transitions_easing_Back::As_mx_transitions_easing_Back(ActionContext* context)
:	ActionObject(context)
{
	setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeIn)));
	setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeInOut)));
	setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeOut)));
}

void As_mx_transitions_easing_Back::Back_easeIn(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeInOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
