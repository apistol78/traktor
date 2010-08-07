#include "Xml/Node.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace xml
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Node", Node, Object)

Node::Node()
:	m_parent(0)
,	m_previousSibling(0)
{
}

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
	T_ASSERT (child->m_parent == 0);

	child->m_parent = this;
	child->m_previousSibling = m_lastChild;
	child->m_nextSibling = 0;
	
	if (m_lastChild != 0)
		m_lastChild->m_nextSibling = child;
	else
		m_firstChild = child;
		
	m_lastChild = child;
}

void Node::removeChild(Node* child)
{
	T_ASSERT (child->m_parent == this);

	if (child->m_previousSibling)
		child->m_previousSibling->m_nextSibling = child->m_nextSibling;

	if (child->m_nextSibling)
		child->m_nextSibling->m_previousSibling = child->m_previousSibling;

	if (m_firstChild == child)
		m_firstChild = child->m_nextSibling;
	if (m_lastChild == child)
		m_lastChild = child->m_previousSibling;

	child->m_parent = 0;
	child->m_previousSibling = 0;
	child->m_nextSibling = 0;
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

void Node::insertBefore(Node* child, Node* beforeNode)
{
	T_ASSERT (child->m_parent == 0);
	T_ASSERT (!beforeNode || beforeNode->m_parent == this);

	if (!beforeNode)
		beforeNode = m_firstChild;

	child->m_parent = this;
	
	child->m_previousSibling = beforeNode ? beforeNode->m_previousSibling : 0;
	child->m_nextSibling = beforeNode;

	if (child->m_previousSibling)
		child->m_previousSibling->m_nextSibling = child;

	if (beforeNode)
		beforeNode->m_previousSibling = child;

	if (m_firstChild == beforeNode)
		m_firstChild = child;
}

void Node::insertAfter(Node* child, Node* afterNode)
{
	T_ASSERT (child->m_parent == 0);
	T_ASSERT (!afterNode || afterNode->m_parent == this);

	if (afterNode)
	{
		child->m_parent = this;
		
		child->m_previousSibling = afterNode;
		child->m_nextSibling = afterNode->m_nextSibling;

		if (child->m_nextSibling)
			child->m_nextSibling->m_previousSibling = child;

		afterNode->m_nextSibling = child;

		if (m_lastChild == afterNode)
			m_lastChild = child;
	}
	else
	{
		T_ASSERT (m_firstChild == 0);
		addChild(child);
	}
}

Node* Node::getParent() const
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

void Node::cloneChildren(Node* clone) const
{
	for (Node* child = m_firstChild; child; child = child->m_nextSibling)
	{
		Ref< Node > childClone = child->cloneUntyped();
		clone->addChild(childClone);
	}
}

	}
}
