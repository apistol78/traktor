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
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();

	T /= D;
	ca.ret = ActionValue(C * T * T * T + B);
}

void As_mx_transitions_easing_Regular::Regular_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();

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
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();

	T = (T / D) - 1.0f;
	ca.ret = ActionValue(C * (T * T * T + 1.0f) + B);
}

	}
}
