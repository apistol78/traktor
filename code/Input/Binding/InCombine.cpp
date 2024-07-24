/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefArray.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor::input
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
	for (const auto& entry : m_entries)
		instance->sourceInstances.push_back(entry.source->createInstance());
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

	for (uint32_t i = 0; i < (uint32_t)m_entries.size(); ++i)
	{
		const float value = m_entries[i].source->evaluate(ici->sourceInstances[i], valueSet, T, dT);
		const float scaled = value * m_entries[i].mul + m_entries[i].add;
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
	s >> MemberAlignedVector< Entry, MemberComposite< Entry > >(L"entries", m_entries);

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
	s >> MemberRef< IInputNode >(L"source", source, AttributePrivate());
	s >> Member< float >(L"mul", mul);
	s >> Member< float >(L"add", add);
}

}
