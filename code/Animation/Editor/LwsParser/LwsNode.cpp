#include "Animation/Editor/LwsParser/LwsNode.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LwsNode", LwsNode, Object)

LwsNode::LwsNode(const std::wstring& name)
:	m_name(name)
{
}

const std::wstring& LwsNode::getName() const
{
	return m_name;
}

	}
}
