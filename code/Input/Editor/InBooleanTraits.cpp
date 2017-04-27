/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Input/Binding/InBoolean.h"
#include "Input/Editor/InBooleanTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InBooleanTraits::getHeader(const IInputNode* node) const
{
	return L"Boolean";
}

std::wstring InBooleanTraits::getDescription(const IInputNode* node) const
{
	const InBoolean* inBoolean = checked_type_cast< const InBoolean*, false >(node);
	switch (inBoolean->m_op)
	{
	case InBoolean::OpNot:
		return L"Not";
	case InBoolean::OpAnd:
		return L"And";
	case InBoolean::OpOr:
		return L"Or";
	case InBoolean::OpXor:
		return L"Xor";
	default:
		return L"";
	}
}

Ref< IInputNode > InBooleanTraits::createNode() const
{
	return new InBoolean();
}

void InBooleanTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InBoolean* inBoolean = checked_type_cast< const InBoolean*, false >(node);

	const RefArray< IInputNode >& sources = inBoolean->m_source;
	for (uint32_t i = 0; i < uint32_t(sources.size()); ++i)
		outInputNodes[toString(i)] = sources[i];

	outInputNodes[L"*"] = 0;
}

void InBooleanTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InBoolean* inBoolean = checked_type_cast< InBoolean*, false >(node);
	if (inputName == L"*")
		inBoolean->m_source.push_back(sourceNode);
	else
	{
		int32_t index = parseString< int32_t >(inputName);
		if (index >= 0 && index < int32_t(inBoolean->m_source.size()))
			inBoolean->m_source[index] = sourceNode;
	}
}

void InBooleanTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InBoolean* inBoolean = checked_type_cast< InBoolean*, false >(node);
	int32_t index = parseString< int32_t >(inputName);
	if (index >= 0 && index < int32_t(inBoolean->m_source.size()))
		inBoolean->m_source.erase(inBoolean->m_source.begin() + index);
}

	}
}
