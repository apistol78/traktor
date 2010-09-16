#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Back.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Back", As_mx_transitions_easing_Back, ActionClass)

As_mx_transitions_easing_Back::As_mx_transitions_easing_Back()
:	ActionClass(L"mx.transitions.easing.Back")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"easeIn", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Back::Back_easeIn)));
	prototype->setMember(L"easeInOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Back::Back_easeInOut)));
	prototype->setMember(L"easeOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Back::Back_easeOut)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_easing_Back::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void As_mx_transitions_easing_Back::Back_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Back::Back_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
