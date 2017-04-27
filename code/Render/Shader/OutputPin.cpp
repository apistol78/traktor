/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{

OutputPin::OutputPin()
{
}

OutputPin::OutputPin(Node* node, const std::wstring& name)
:	m_node(node)
,	m_name(name)
{
}

Node* OutputPin::getNode() const
{
	return m_node;
}

const std::wstring& OutputPin::getName() const
{
	return m_name;
}

	}
}
