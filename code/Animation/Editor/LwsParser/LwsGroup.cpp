/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/LwsParser/LwsGroup.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LwsGroup", LwsGroup, LwsNode)

LwsGroup::LwsGroup(const std::wstring& name)
:	LwsNode(name)
{
}

void LwsGroup::add(LwsNode* node)
{
	m_nodes.push_back(node);
}

uint32_t LwsGroup::getCount() const
{
	return uint32_t(m_nodes.size());
}

Ref< const LwsNode > LwsGroup::get(uint32_t index) const
{
	T_ASSERT (index < m_nodes.size());
	return m_nodes[index];
}

Ref< const LwsNode > LwsGroup::find(const std::wstring& name, uint32_t from) const
{
	if (m_nodes.empty() || from >= m_nodes.size())
		return 0;

	for (uint32_t i = from; i < m_nodes.size(); ++i)
	{
		if (m_nodes[i]->getName() == name)
			return m_nodes[i];
	}

	return 0;
}

	}
}
