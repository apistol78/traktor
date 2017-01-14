#include "Core/RefArray.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InCombineInstance : public RefCountImpl< IInputNode::Instance >
{
	RefArray< IInputNode::Instance > sourceInstances;
};
		
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InCombine", 1, InCombine, IInputNode)

InCombine::InCombine()
:	m_operator(CoAdd)
{
}

Ref< IInputNode::Instance > InCombine::createInstance() const
{
	Ref< InCombineInstance > instance = new InCombineInstance();
	for (std::vector< Entry >::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
		instance->sourceInstances.push_back(i->source->createInstance());
	return instance;
}

float InCombine::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InCombineInstance* ici = static_cast< InCombineInstance* >(instance);
	float result = 0.0f;
	
	for (uint32_t i = 0; i < m_entries.size(); ++i)
	{
		float value = m_entries[i].source->evaluate(ici->sourceInstances[i], valueSet, T, dT);
		float scaled = value * m_entries[i].mul + m_entries[i].add;

		if (i > 0)
		{
			switch (m_operator)
			{
			case CoAdd:
				result += scaled;
				break;

			case CoSub:
				result -= scaled;
				break;

			case CoMul:
				result *= scaled;
				break;

			case CoDiv:
				result /= scaled;
				break;
			}
		}
		else
			result = scaled;
	}
	
	return result;
}

void InCombine::serialize(ISerializer& s)
{
	s >> MemberStlVector< Entry, MemberComposite< Entry > >(L"entries", m_entries);
	
	if (s.getVersion() >= 1)
	{
		const MemberEnum< CombineOperator >::Key c_CombineOperator_Keys[] =
		{
			{ L"CoAdd", CoAdd },
			{ L"CoSub", CoSub },
			{ L"CoMul", CoMul },
			{ L"CoDiv", CoDiv },
			{ 0 }
		};
		s >> MemberEnum< CombineOperator >(L"operator", m_operator, c_CombineOperator_Keys);
	}
}

void InCombine::Entry::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", source);
	s >> Member< float >(L"mul", mul);
	s >> Member< float >(L"add", add);
}
	
	}
}
