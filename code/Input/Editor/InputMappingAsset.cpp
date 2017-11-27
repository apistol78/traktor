/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefSet.h"
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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingAsset", 1, InputMappingAsset, ISerializable)

void InputMappingAsset::addInputNode(IInputNode* inputNode)
{
	m_inputNodes.insert(inputNode);
}

void InputMappingAsset::removeInputNode(IInputNode* inputNode)
{
	m_inputNodes.erase(inputNode);
}

const RefSet< IInputNode >& InputMappingAsset::getInputNodes() const
{
	return m_inputNodes;
}

void InputMappingAsset::setPosition(const Object* object, const Position& position)
{
	m_positions[object] = position;
}

InputMappingAsset::Position InputMappingAsset::getPosition(const Object* object) const
{
	const static Position c_zero = { 0 };
	std::map< Ref< const Object >, Position >::const_iterator it = m_positions.find(object);
	return it != m_positions.end() ? it->second : c_zero;
}

void InputMappingAsset::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
	{
		s >> MemberRefSet< IInputNode >(L"inputNodes", m_inputNodes);
		s >> MemberStlMap< Ref< const Object >, InputMappingAsset::Position, MemberStlPair< Ref< const Object >, InputMappingAsset::Position, MemberRef< const Object >, MemberPosition > >(L"positions", m_positions);
	}

	s >> MemberRef< InputMappingSourceData >(L"sourceData", m_sourceData);
	s >> MemberRef< InputMappingStateData >(L"stateData", m_stateData);
	s >> MemberStlList< Guid >(L"dependencies", m_dependencies);
}

	}
}
