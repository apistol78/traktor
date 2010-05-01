#include <cmath>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMath.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMath", AsMath, ActionClass)

Ref< AsMath > AsMath::getInstance()
{
	static Ref< AsMath > instance = 0;
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
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"e", ActionValue(avm_number_t(2.7182818284590452354)));
	prototype->setMember(L"ln2", ActionValue(avm_number_t(0.69314718055994530942)));
	prototype->setMember(L"log2e", ActionValue(avm_number_t(1.4426950408889634074)));
	prototype->setMember(L"ln10", ActionValue(avm_number_t(2.30258509299404568402)));
	prototype->setMember(L"log10e", ActionValue(avm_number_t(0.43429448190325182765)));
	prototype->setMember(L"pi", ActionValue(avm_number_t(3.14159265358979323846)));
	prototype->setMember(L"sqrt1_2", ActionValue(avm_number_t(0.7071067811865475244)));
	prototype->setMember(L"sqrt2", ActionValue(avm_number_t(1.4142135623730950488)));
	prototype->setMember(L"abs", ActionValue(createNativeFunction(this, &AsMath::Math_abs)));
	prototype->setMember(L"acos", ActionValue(createNativeFunction(this, &AsMath::Math_acos)));
	prototype->setMember(L"asin", ActionValue(createNativeFunction(this, &AsMath::Math_asin)));
	prototype->setMember(L"atan", ActionValue(createNativeFunction(this, &AsMath::Math_atan)));
	prototype->setMember(L"atan2", ActionValue(createNativeFunction(this, &AsMath::Math_atan2)));
	prototype->setMember(L"ceil", ActionValue(createNativeFunction(this, &AsMath::Math_ceil)));
	prototype->setMember(L"cos", ActionValue(createNativeFunction(this, &AsMath::Math_cos)));
	prototype->setMember(L"exp", ActionValue(createNativeFunction(this, &AsMath::Math_exp)));
	prototype->setMember(L"floor", ActionValue(createNativeFunction(this, &AsMath::Math_floor)));
	prototype->setMember(L"log", ActionValue(createNativeFunction(this, &AsMath::Math_log)));
	prototype->setMember(L"max", ActionValue(createNativeFunction(this, &AsMath::Math_max)));
	prototype->setMember(L"min", ActionValue(createNativeFunction(this, &AsMath::Math_min)));
	prototype->setMember(L"pow", ActionValue(createNativeFunction(this, &AsMath::Math_pow)));
	prototype->setMember(L"random", ActionValue(createNativeFunction(this, &AsMath::Math_random)));
	prototype->setMember(L"round", ActionValue(createNativeFunction(this, &AsMath::Math_round)));
	prototype->setMember(L"sin", ActionValue(createNativeFunction(this, &AsMath::Math_sin)));
	prototype->setMember(L"sqrt", ActionValue(createNativeFunction(this, &AsMath::Math_sqrt)));
	prototype->setMember(L"tan", ActionValue(createNativeFunction(this, &AsMath::Math_tan)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsMath::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsMath::Math_abs(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::fabs(n));
}

void AsMath::Math_acos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::acos(n));
}

void AsMath::Math_asin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::asin(n));
}

void AsMath::Math_atan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::atan(n));
}

void AsMath::Math_atan2(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumberSafe();
	avm_number_t y = ca.args[1].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::atan2(x, y));
}

void AsMath::Math_ceil(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::ceil(n));
}

void AsMath::Math_cos(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::cos(n));
}

void AsMath::Math_exp(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::exp(n));
}

void AsMath::Math_floor(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::floor(n));
}

void AsMath::Math_log(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::log(n));
}

void AsMath::Math_max(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumberSafe();
		if (n > n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_min(CallArgs& ca)
{
	avm_number_t n1 = ca.args[0].getNumberSafe();
	for (size_t i = 1; i < ca.args.size(); ++i)
	{
		avm_number_t n = ca.args[i].getNumberSafe();
		if (n < n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_pow(CallArgs& ca)
{
	avm_number_t b = ca.args[0].getNumberSafe();
	avm_number_t e = ca.args[1].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::pow(b, e));
}

void AsMath::Math_random(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(m_random.nextDouble()));
}

void AsMath::Math_round(CallArgs& ca)
{
	avm_number_t x = ca.args[0].getNumberSafe();
	avm_number_t f = (avm_number_t)std::abs(x);

	if (f - int(f) >= 0.5)
		f += 1.0;

	if (x < 0)
		f = -f;

	ca.ret = ActionValue(f);
}

void AsMath::Math_sin(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::sin(n));
}

void AsMath::Math_sqrt(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::sqrt(n));
}

void AsMath::Math_tan(CallArgs& ca)
{
	avm_number_t n = ca.args[0].getNumberSafe();
	ca.ret = ActionValue((avm_number_t)std::tan(n));
}

	}
}
