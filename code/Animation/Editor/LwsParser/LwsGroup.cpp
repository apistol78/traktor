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

Ref< const LwsNode > LwsGroup::find(const std::wstring& name) const
{
	for (RefArray< LwsNode >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

	}
}
