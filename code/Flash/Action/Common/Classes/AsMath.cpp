/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
,	m_random(std::clock())
{
	setMember("e", ActionValue(float(2.7182818284590452354)));
	setMember("ln2", ActionValue(float(0.69314718055994530942)));
	setMember("log2e", ActionValue(float(1.4426950408889634074)));
	setMember("ln10", ActionValue(float(2.30258509299404568402)));
	setMember("log10e", ActionValue(float(0.43429448190325182765)));
	setMember("pi", ActionValue(float(3.14159265358979323846)));
	setMember("PI", ActionValue(float(3.14159265358979323846)));
	setMember("sqrt1_2", ActionValue(float(0.7071067811865475244)));
	setMember("sqrt2", ActionValue(float(1.4142135623730950488)));
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
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::fabs(n));
}

void AsMath::Math_acos(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::acos(n));
}

void AsMath::Math_asin(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::asin(n));
}

void AsMath::Math_atan(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::atan(n));
}

void AsMath::Math_atan2(CallArgs& ca)
{
	float x = ca.args[0].getFloat();
	float y = ca.args[1].getFloat();
	ca.ret = ActionValue((float)std::atan2(x, y));
}

void AsMath::Math_ceil(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::ceil(n));
}

void AsMath::Math_cos(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::cos(n));
}

void AsMath::Math_exp(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::exp(n));
}

void AsMath::Math_floor(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::floor(n));
}

void AsMath::Math_log(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::log(n));
}

void AsMath::Math_max(CallArgs& ca)
{
	float n1 = ca.args[0].getFloat();
	for (uint32_t i = 1; i < ca.args.size(); ++i)
	{
		float n = ca.args[i].getFloat();
		if (n > n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_min(CallArgs& ca)
{
	float n1 = ca.args[0].getFloat();
	for (uint32_t i = 1; i < ca.args.size(); ++i)
	{
		float n = ca.args[i].getFloat();
		if (n < n1)
			n1 = n;
	}
	ca.ret = ActionValue(n1);
}

void AsMath::Math_pow(CallArgs& ca)
{
	float b = ca.args[0].getFloat();
	float e = ca.args[1].getFloat();
	ca.ret = ActionValue((float)std::pow(b, e));
}

void AsMath::Math_random(CallArgs& ca)
{
	ca.ret = ActionValue(float(m_random.nextDouble()));
}

void AsMath::Math_round(CallArgs& ca)
{
	float x = ca.args[0].getFloat();
	float f = (float)std::abs(x);

	f = int32_t(f + 0.5f);
	if (x < 0)
		f = -f;

	ca.ret = ActionValue(f);
}

void AsMath::Math_sin(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::sin(n));
}

void AsMath::Math_sqrt(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::sqrt(n));
}

void AsMath::Math_tan(CallArgs& ca)
{
	float n = ca.args[0].getFloat();
	ca.ret = ActionValue((float)std::tan(n));
}

	}
}
