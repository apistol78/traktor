#include "Xml/Node.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace xml
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Node", Node, Object)

std::wstring Node::getName() const
{
	return L"";
}

void Node::setName(const std::wstring& name)
{
}

std::wstring Node::getValue() const
{
	return L"";
}

void Node::setValue(const std::wstring& value)
{
}

void Node::write(OutputStream& os) const
{
	for (Ref< Node > child = m_firstChild; child != 0; child = child->m_nextSibling)
		child->write(os);
}

void Node::addChild(Node* child)
{
	child->m_parent = this;
	child->m_previousSibling = m_lastChild;
	child->m_nextSibling = 0;
	
	if (m_lastChild != 0)
		m_lastChild->m_nextSibling = child;
	else
		m_firstChild = child;
		
	m_lastChild = child;
}

void Node::removeAllChildren()
{
	for (Ref< Node > child = m_firstChild; child; child = child->m_nextSibling)
	{
		T_ASSERT (child->m_parent == this);
		child->m_parent = 0;
	}
	m_firstChild = 0;
	m_lastChild = 0;
}

void Node::insertBefore(Node* child, Node* node)
{
	// TODO Implement and alter addChild.
}

void Node::insertAfter(Node* child, Node* node)
{
	// TODO Implement and alter addChild.
}

Ref< Node > Node::getParent() const
{
	return m_parent;
}

Ref< Node > Node::getPreviousSibling() const
{
	return m_previousSibling;
}

Ref< Node > Node::getNextSibling() const
{
	return m_nextSibling;
}

Ref< Node > Node::getFirstChild() const
{
	return m_firstChild;
}

Ref< Node > Node::getLastChild() const
{
	return m_lastChild;
}

	}
}
