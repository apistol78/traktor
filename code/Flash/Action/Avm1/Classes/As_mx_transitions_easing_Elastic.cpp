#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/As_mx_transitions_easing_Elastic.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Elastic", As_mx_transitions_easing_Elastic, ActionClass)

Ref< As_mx_transitions_easing_Elastic > As_mx_transitions_easing_Elastic::getInstance()
{
	static Ref< As_mx_transitions_easing_Elastic > instance;
	if (!instance)
	{
		instance = new As_mx_transitions_easing_Elastic();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

As_mx_transitions_easing_Elastic::As_mx_transitions_easing_Elastic()
:	ActionClass(L"Elastic")
{
}

void As_mx_transitions_easing_Elastic::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"easeIn", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Elastic::Elastic_easeIn)));
	prototype->setMember(L"easeInOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Elastic::Elastic_easeInOut)));
	prototype->setMember(L"easeOut", ActionValue(createNativeFunction(this, &As_mx_transitions_easing_Elastic::Elastic_easeOut)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue As_mx_transitions_easing_Elastic::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void As_mx_transitions_easing_Elastic::Elastic_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Elastic::Elastic_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Elastic::Elastic_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumberSafe();
	avm_number_t B = ca.args[1].getNumberSafe();
	avm_number_t C = ca.args[2].getNumberSafe();
	avm_number_t D = ca.args[3].getNumberSafe();

	if (T <= 0.0f)
	{
		ca.ret = ActionValue(B);
		return;
	}

	avm_number_t t = T / D;
	if (t >= 1.0f)
	{
		ca.ret = ActionValue(B + C);
		return;
	}

	avm_number_t p = D * 0.3f;
	avm_number_t s = p / 4.0f;

	ca.ret = ActionValue(C * powf(2, -10.0f * t) * sinf((t * D - s) * TWO_PI / p) + C + B);
}

	}
}
