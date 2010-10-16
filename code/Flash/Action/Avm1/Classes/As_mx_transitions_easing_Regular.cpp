#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Regular.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Regular", As_mx_transitions_easing_Regular, ActionClass)

As_mx_transitions_easing_Regular::As_mx_transitions_easing_Regular()
:	ActionClass("mx.transitions.easing.Regular")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("easeIn", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Regular::Regular_easeIn)));
	prototype->setMember("easeInOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Regular::Regular_easeInOut)));
	prototype->setMember("easeOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Regular::Regular_easeOut)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_easing_Regular::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void As_mx_transitions_easing_Regular::Regular_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();

	T /= D;
	ca.ret = ActionValue(C * T * T * T + B);
}

void As_mx_transitions_easing_Regular::Regular_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();

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
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();

	T = (T / D) - 1.0f;
	ca.ret = ActionValue(C * (T * T * T + 1.0f) + B);
}

	}
}
