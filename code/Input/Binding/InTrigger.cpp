#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InTrigger.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InTrigger", 0, InTrigger, IInputNode)

InTrigger::InTrigger()
:	m_flank(FlPositive)
,	m_duration(0.0f)
{
}

InTrigger::InTrigger(IInputNode* source, Flank flank, float duration)
:	m_source(source)
,	m_flank(flank)
,	m_duration(duration)
{
}

InputValue InTrigger::evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const
{
	InputValue value = m_source->evaluate(valueSet, T, dT, currentStateValue);
	
	float v = value.get();
	float vT = value.getTime();
	
	float vT0 = T - vT;
	if (vT0 > m_duration)
		return InputValue(0.0f, vT);

	bool pulse = false;
	
	if (m_flank == FlPositive)
		pulse = v > 0.5f;
	else if (m_flank == FlNegative)
		pulse = v < 0.5f;
	
	if (pulse)
		return InputValue(1.0f, vT);
	else
		return InputValue(0.0f, vT);
}

bool InTrigger::serialize(ISerializer& s)
{
	const MemberEnum< Flank >::Key c_Flank_Keys[] =
	{
		{ L"FlPositive", FlPositive },
		{ L"FlNegative", FlNegative },
		0
	};

	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberEnum< Flank >(L"flank", m_flank, c_Flank_Keys);
	s >> Member< float >(L"duration", m_duration);
	
	return true;
}
	
	}
}
