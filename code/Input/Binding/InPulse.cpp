#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InPulse.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InPulse", 0, InPulse, IInputNode)

InPulse::InPulse()
:	m_delay(0.0f)
,	m_interval(0.0f)
{
}

InputValue InPulse::evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const
{
	InputValue value = m_source->evaluate(valueSet, T, dT, currentStateValue);
	
	float v = value.get();
	float vT = value.getTime();
	
	if (v < 0.5f)
		return InputValue(0.0f, vT);

	float vT0 = vT - m_delay;
	if (vT0 <= 0.0f)
		return InputValue(1.0f, vT);
	
	int32_t i = int32_t(vT0 / m_interval);
	if ((i & 1) == 0)
		return InputValue(0.0f, vT);
	else
		return InputValue(1.0f, vT);
}

bool InPulse::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> Member< float >(L"delay", m_delay);
	s >> Member< float >(L"interval", m_interval);
	return true;
}
	
	}
}
