#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Back.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Back", As_mx_transitions_easing_Back, ActionClass)

As_mx_transitions_easing_Back::As_mx_transitions_easing_Back(ActionContext* context)
:	ActionClass(context, "mx.transitions.easing.Back")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeIn)));
	prototype->setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeInOut)));
	prototype->setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Back::Back_easeOut)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_mx_transitions_easing_Back::init(ActionObject* self, const ActionValueArray& args)
{
}

void As_mx_transitions_easing_Back::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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
