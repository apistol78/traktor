#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_mx_transitions_easing_Elastic.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_mx_transitions_easing_Elastic", As_mx_transitions_easing_Elastic, ActionObject)

As_mx_transitions_easing_Elastic::As_mx_transitions_easing_Elastic(ActionContext* context)
:	ActionObject(context)
{
	setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Elastic::Elastic_easeIn)));
	setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Elastic::Elastic_easeInOut)));
	setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_Elastic::Elastic_easeOut)));
}

void As_mx_transitions_easing_Elastic::Elastic_easeIn(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Elastic::Elastic_easeInOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_Elastic::Elastic_easeOut(CallArgs& ca)
{
	avm_number_t T = ca.args[0].getNumber();
	avm_number_t B = ca.args[1].getNumber();
	avm_number_t C = ca.args[2].getNumber();
	avm_number_t D = ca.args[3].getNumber();

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
