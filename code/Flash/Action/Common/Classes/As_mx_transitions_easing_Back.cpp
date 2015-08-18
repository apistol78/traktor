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
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
