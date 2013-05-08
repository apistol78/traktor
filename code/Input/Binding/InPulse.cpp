#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InPulse.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InPulseInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float previousValue;
	float issueTime;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InPulse", 0, InPulse, IInputNode)

InPulse::InPulse()
:	m_delay(0.0f)
,	m_interval(0.0f)
{
}

InPulse::InPulse(IInputNode* source, float delay, float interval)
:	m_source(source)
,	m_delay(delay)
,	m_interval(interval)
{
}

Ref< IInputNode::Instance > InPulse::createInstance() const
{
	Ref< InPulseInstance > instance = new InPulseInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->previousValue = 0.0f;
	instance->issueTime = 0.0f;
	return instance;
}

float InPulse::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InPulseInstance* ipi = static_cast< InPulseInstance* >(instance);

	float V = m_source->evaluate(ipi->sourceInstance, valueSet, T, dT);
	float dV = V - ipi->previousValue;

	ipi->previousValue = V;
	
	if (!asBoolean(V))
		return asFloat(false);

	if (dV > FUZZY_EPSILON)
		ipi->issueTime = T;
	
	float T0 = T - ipi->issueTime - m_delay;
	if (T0 < 0.0f)
		return asFloat(true);
	
	int32_t i = int32_t(T0 / m_interval);
	if ((i & 1) == 0)
		return asFloat(false);
	else
		return asFloat(true);
}

void InPulse::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> Member< float >(L"delay", m_delay);
	s >> Member< float >(L"interval", m_interval);
}
	
	}
}
