/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
	for (AlignedVector< ObjectPosition >::iterator i = m_positions.begin(); i != m_positions.end(); ++i)
	{
		if (i->object == object)
		{
			i->position = position;
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
	for (AlignedVector< ObjectPosition >::const_iterator i = m_positions.begin(); i != m_positions.end(); ++i)
	{
		if (i->object == object)
			return i->position;
	}
	return c_zero;
}

void InputMappingAsset::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberRefArray< IInputNode >(L"inputNodes", m_inputNodes);

	if (s.getVersion() >= 2)
		s >> MemberAlignedVector< ObjectPosition, MemberComposite< ObjectPosition > >(L"positions", m_positions);
	else if (s.getVersion() >= 1)
	{
		std::map< Ref< const Object >, Position > positions;
		s >> MemberStlMap< Ref< const Object >, InputMappingAsset::Position, MemberStlPair< Ref< const Object >, InputMappingAsset::Position, MemberRef< const Object >, MemberPosition > >(L"positions", positions);
		for (std::map< Ref< const Object >, Position >::const_iterator i = positions.begin(); i != positions.end(); ++i)
			m_positions.push_back(ObjectPosition(i->first, i->second));
	}

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
