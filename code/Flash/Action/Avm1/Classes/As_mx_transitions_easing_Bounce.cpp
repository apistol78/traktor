#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Bounce.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Bounce", As_mx_transitions_easing_Bounce, ActionClass)

As_mx_transitions_easing_Bounce::As_mx_transitions_easing_Bounce(ActionContext* context)
:	ActionClass(context, "mx.transitions.easing.Bounce")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeIn)));
	prototype->setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeInOut)));
	prototype->setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Bounce::Bounce_easeOut)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_mx_transitions_easing_Bounce::init(ActionObject* self, const ActionValueArray& args) const
{
}

void As_mx_transitions_easing_Bounce::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void As_mx_transitions_easing_Bounce::Bounce_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Bounce::Bounce_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Bounce::Bounce_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
