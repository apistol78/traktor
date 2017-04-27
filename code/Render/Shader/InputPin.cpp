/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"

namespace traktor
{
	namespace render
	{

InputPin::InputPin(Node* node, const std::wstring& name, bool optional)
:	m_node(node)
,	m_name(name)
,	m_optional(optional)
{
}

Node* InputPin::getNode() const
{
	return m_node;
}

const std::wstring& InputPin::getName() const
{
	return m_name;
}

bool InputPin::isOptional() const
{
	return m_optional;
}

	}
}
