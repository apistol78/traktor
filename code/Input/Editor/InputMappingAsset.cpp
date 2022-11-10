/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Editor/InputMappingAsset.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

class MemberPosition : public MemberComplex
{
public:
	MemberPosition(const wchar_t* const name, InputMappingAsset::Position& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< int32_t >(L"x", m_ref.x);
		s >> Member< int32_t >(L"y", m_ref.y);
	}

private:
	InputMappingAsset::Position& m_ref;
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingAsset", 2, InputMappingAsset, ISerializable)

void InputMappingAsset::addInputNode(IInputNode* inputNode)
{
	if (std::find(m_inputNodes.begin(), m_inputNodes.end(), inputNode) == m_inputNodes.end())
		m_inputNodes.push_back(inputNode);
}

void InputMappingAsset::removeInputNode(IInputNode* inputNode)
{
	m_inputNodes.remove(inputNode);
}

const RefArray< IInputNode >& InputMappingAsset::getInputNodes() const
{
	return m_inputNodes;
}

void InputMappingAsset::setPosition(const Object* object, const Position& position)
{
	for (auto& op : m_positions)
	{
		if (op.object == object)
		{
			op.position = position;
			return;
		}
	}

	ObjectPosition& op = m_positions.push_back();
	op.object = object;
	op.position = position;
}

InputMappingAsset::Position InputMappingAsset::getPosition(const Object* object) const
{
	const static Position c_zero = { 0 };
	for (const auto& op : m_positions)
	{
		if (op.object == object)
			return op.position;
	}
	return c_zero;
}

void InputMappingAsset::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 2);

	s >> MemberRefArray< IInputNode >(L"inputNodes", m_inputNodes);
	s >> MemberAlignedVector< ObjectPosition, MemberComposite< ObjectPosition > >(L"positions", m_positions);
	s >> MemberRef< InputMappingSourceData >(L"sourceData", m_sourceData);
	s >> MemberRef< InputMappingStateData >(L"stateData", m_stateData);
	s >> MemberStlList< Guid >(L"dependencies", m_dependencies);
}

InputMappingAsset::ObjectPosition::ObjectPosition()
{
}

InputMappingAsset::ObjectPosition::ObjectPosition(const Object* object_, const Position& position_)
:	object(object_)
,	position(position_)
{
}

void InputMappingAsset::ObjectPosition::serialize(ISerializer& s)
{
	s >> MemberRef< const Object >(L"object", object);
	s >> MemberPosition(L"position", position);
}

	}
}
