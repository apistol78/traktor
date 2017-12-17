/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Html/Node.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Node", Node, Object)

Node::Node()
:	m_parent(0)
,	m_previousSibling(0)
{
}

std::wstring Node::getName() const
{
	return L"";
}

std::wstring Node::getValue() const
{
	return L"";
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

void Node::insertBefore(Node* child, Node* node)
{
	// TODO Implement and alter addChild.
}

void Node::insertAfter(Node* child, Node* node)
{
	// TODO Implement and alter addChild.
}

Node* Node::getParent() const
{
	return m_parent;
}

Node* Node::getPreviousSibling() const
{
	return m_previousSibling;
}

Node* Node::getNextSibling() const
{
	return m_nextSibling;
}

Node* Node::getFirstChild() const
{
	return m_firstChild;
}

Node* Node::getLastChild() const
{
	return m_lastChild;
}

void Node::toString(OutputStream& os) const
{
	for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
		child->toString(os);
}

	}
}
