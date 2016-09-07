#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InLowPass.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InLowPassInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float filteredValue;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InLowPass", 0, InLowPass, IInputNode)

InLowPass::InLowPass()
:	m_coeff(0.0f)
{
}

InLowPass::InLowPass(IInputNode* source, float coeff)
:	m_source(source)
,	m_coeff(coeff)
{
}

Ref< IInputNode::Instance > InLowPass::createInstance() const
{
	Ref< InLowPassInstance > instance = new InLowPassInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->filteredValue = 0.0f;
	return instance;
}

float InLowPass::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InLowPassInstance* lpi = static_cast< InLowPassInstance* >(instance);
	float V = m_source->evaluate(lpi->sourceInstance, valueSet, T, dT);
	lpi->filteredValue = V * m_coeff + lpi->filteredValue * (1.0f - m_coeff);
	return lpi->filteredValue;
}

void InLowPass::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> Member< float >(L"coeff", m_coeff);
}
	
	}
}
