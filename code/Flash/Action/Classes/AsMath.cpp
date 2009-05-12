#include <cmath>
#include "Flash/Action/Classes/AsMath.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionContext.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMath", AsMath, ActionClass)

AsMath* AsMath::getInstance()
{
	static AsMath* instance = 0;
	if (!instance)
	{
		instance = new AsMath();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsMath::AsMath()
:	ActionClass(L"Math")
{
}

void AsMath::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"e", ActionValue(2.7182818284590452354));
	prototype->setMember(L"ln2", ActionValue(0.69314718055994530942));
	prototype->setMember(L"log2e", ActionValue(1.4426950408889634074));
	prototype->setMember(L"ln10", ActionValue(2.30258509299404568402));
	prototype->setMember(L"log10e", ActionValue(0.43429448190325182765));
	prototype->setMember(L"pi", ActionValue(3.14159265358979323846));
	prototype->setMember(L"sqrt1_2", ActionValue(0.7071067811865475244));
	prototype->setMember(L"sqrt2", ActionValue(1.4142135623730950488));
	prototype->setMember(L"abs", createNativeFunctionValue(this, &AsMath::Math_abs));
	prototype->setMember(L"acos", createNativeFunctionValue(this, &AsMath::Math_acos));
	prototype->setMember(L"asin", createNativeFunctionValue(this, &AsMath::Math_asin));
	prototype->setMember(L"atan", createNativeFunctionValue(this, &AsMath::Math_atan));
	prototype->setMember(L"atan2", createNativeFunctionValue(this, &AsMath::Math_atan2));
	prototype->setMember(L"ceil", createNativeFunctionValue(this, &AsMath::Math_ceil));
	prototype->setMember(L"cos", createNativeFunctionValue(this, &AsMath::Math_cos));
	prototype->setMember(L"exp", createNativeFunctionValue(this, &AsMath::Math_exp));
	prototype->setMember(L"floor", createNativeFunctionValue(this, &AsMath::Math_floor));
	prototype->setMember(L"log", createNativeFunctionValue(this, &AsMath::Math_log));
	prototype->setMember(L"max", createNativeFunctionValue(this, &AsMath::Math_max));
	prototype->setMember(L"min", createNativeFunctionValue(this, &AsMath::Math_min));
	prototype->setMember(L"pow", createNativeFunctionValue(this, &AsMath::Math_pow));
	prototype->setMember(L"random", createNativeFunctionValue(this, &AsMath::Math_random));
	prototype->setMember(L"round", createNativeFunctionValue(this, &AsMath::Math_round));
	prototype->setMember(L"sin", createNativeFunctionValue(this, &AsMath::Math_sin));
	prototype->setMember(L"sqrt", createNativeFunctionValue(this, &AsMath::Math_sqrt));
	prototype->setMember(L"tan", createNativeFunctionValue(this, &AsMath::Math_tan));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsMath::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsMath::Math_abs(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::fabs(n));
}

void AsMath::Math_acos(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::acos(n));
}

void AsMath::Math_asin(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::asin(n));
}

void AsMath::Math_atan(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::atan(n));
}

void AsMath::Math_atan2(CallArgs& ca)
{
	double x = ca.args[0].getNumberSafe();
	double y = ca.args[1].getNumberSafe();
	ca.ret = ActionValue(std::atan2(x, y));
}

void AsMath::Math_ceil(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::ceil(n));
}

void AsMath::Math_cos(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::cos(n));
}

void AsMath::Math_exp(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::exp(n));
}

void AsMath::Math_floor(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::floor(n));
}

void AsMath::Math_log(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::log(n));
}

void AsMath::Math_max(CallArgs& ca)
{
	double n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		double n = ca.args[i].getNumberSafe();
		if (n > n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_min(CallArgs& ca)
{
	double n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		double n = ca.args[i].getNumberSafe();
		if (n < n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_pow(CallArgs& ca)
{
	double b = ca.args[0].getNumberSafe();
	double e = ca.args[1].getNumberSafe();
	ca.ret = ActionValue(std::pow(b, e));
}

void AsMath::Math_random(CallArgs& ca)
{
	ca.ret = ActionValue(m_random.nextDouble());
}

void AsMath::Math_round(CallArgs& ca)
{
	double x = ca.args[0].getNumberSafe();
	double f = abs(x);

	if (f - int(f) >= 0.5)
		f += 1.0;

	if (x < 0)
		f = -f;

	ca.ret = ActionValue(f);
}

void AsMath::Math_sin(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::sin(n));
}

void AsMath::Math_sqrt(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::sqrt(n));
}

void AsMath::Math_tan(CallArgs& ca)
{
	double n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue(std::tan(n));
}

	}
}
