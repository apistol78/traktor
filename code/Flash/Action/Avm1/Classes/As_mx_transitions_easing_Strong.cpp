#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Strong.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Strong", As_mx_transitions_easing_Strong, ActionClass)

As_mx_transitions_easing_Strong::As_mx_transitions_easing_Strong()
:	ActionClass("mx.transitions.easing.Strong")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("easeIn", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Strong::Strong_easeIn)));
	prototype->setMember("easeInOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Strong::Strong_easeInOut)));
	prototype->setMember("easeOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Strong::Strong_easeOut)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_easing_Strong::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void As_mx_transitions_easing_Strong::Strong_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Strong::Strong_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Strong::Strong_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
