#include <cmath>
#include <ctime>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsMath.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMath", AsMath, ActionObject)

AsMath::AsMath(ActionContext* context)
:	ActionObject(context)
#if !defined(WINCE)
,	m_random(std::clock())
#endif
{
	setMember("e", ActionValue(avm_number_t(2.7182818284590452354)));
	setMember("ln2", ActionValue(avm_number_t(0.69314718055994530942)));
	setMember("log2e", ActionValue(avm_number_t(1.4426950408889634074)));
	setMember("ln10", ActionValue(avm_number_t(2.30258509299404568402)));
	setMember("log10e", ActionValue(avm_number_t(0.43429448190325182765)));
	setMember("pi", ActionValue(avm_number_t(3.14159265358979323846)));
	setMember("PI", ActionValue(avm_number_t(3.14159265358979323846)));
	setMember("sqrt1_2", ActionValue(avm_number_t(0.7071067811865475244)));
	setMember("sqrt2", ActionValue(avm_number_t(1.4142135623730950488)));
	setMember("abs", ActionValue(createNativeFunction(context, this, &AsMath::Math_abs)));
	setMember("acos", ActionValue(createNativeFunction(context, this, &AsMath::Math_acos)));
	setMember("asin", ActionValue(createNativeFunction(context, this, &AsMath::Math_asin)));
	setMember("atan", ActionValue(createNativeFunction(context, this, &AsMath::Math_atan)));
	setMember("atan2", ActionValue(createNativeFunction(context, this, &AsMath::Math_atan2)));
	setMember("ceil", ActionValue(createNativeFunction(context, this, &AsMath::Math_ceil)));
	setMember("cos", ActionValue(createNativeFunction(context, this, &AsMath::Math_cos)));
	setMember("exp", ActionValue(createNativeFunction(context, this, &AsMath::Math_exp)));
	setMember("floor", ActionValue(createNativeFunction(context, this, &AsMath::Math_floor)));
	setMember("log", ActionValue(createNativeFunction(context, this, &AsMath::Math_log)));
	setMember("max", ActionValue(createNativeFunction(context, this, &AsMath::Math_max)));
	setMember("min", ActionValue(createNativeFunction(context, this, &AsMath::Math_min)));
	setMember("pow", ActionValue(createNativeFunction(context, this, &AsMath::Math_pow)));
	setMember("random", ActionValue(createNativeFunction(context, this, &AsMath::Math_random)));
	setMember("round", ActionValue(createNativeFunction(context, this, &AsMath::Math_round)));
	setMember("sin", ActionValue(createNativeFunction(context, this, &AsMath::Math_sin)));
	setMember("sqrt", ActionValue(createNativeFunction(context, this, &AsMath::Math_sqrt)));
	setMember("tan", ActionValue(createNativeFunction(context, this, &AsMath::Math_tan)));
}

void AsMath::Math_abs(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::fabs(n));
}

void AsMath::Math_acos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::acos(n));
}

void AsMath::Math_asin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::asin(n));
}

void AsMath::Math_atan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::atan(n));
}

void AsMath::Math_atan2(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumber();
	avm_number_t y = ca.args[1].getNumber();
	ca.ret = ActionValue((avm_number_t)std::atan2(x, y));
}

void AsMath::Math_ceil(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::ceil(n));
}

void AsMath::Math_cos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::cos(n));
}

void AsMath::Math_exp(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::exp(n));
}

void AsMath::Math_floor(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::floor(n));
}

void AsMath::Math_log(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::log(n));
}

void AsMath::Math_max(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumber();
	for (uint32_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumber();
		if (n > n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_min(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumber();
	for (uint32_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumber();
		if (n < n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_pow(CallArgs& ca)
{
	avm_number_t b = ca.args[0].getNumber();
	avm_number_t e = ca.args[1].getNumber();
	ca.ret = ActionValue((avm_number_t)std::pow(b, e));
}

void AsMath::Math_random(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(m_random.nextDouble()));
}

void AsMath::Math_round(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumber();
	avm_number_t f = (avm_number_t)std::abs(x);

	f = int32_t(f + 0.5f);
	if (x < 0)
		f = -f;

	ca.ret = ActionValue(f);
}

void AsMath::Math_sin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::sin(n));
}

void AsMath::Math_sqrt(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::sqrt(n));
}

void AsMath::Math_tan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumber();
	ca.ret = ActionValue((avm_number_t)std::tan(n));
}

	}
}
