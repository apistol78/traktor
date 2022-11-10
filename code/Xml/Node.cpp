/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Xml/Node.h"

namespace traktor::xml
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Node", Node, Object)

Node::~Node()
{
	// Release children non-recursively in order to prevent excessive recursion.
	if (m_lastChild)
	{
		for (Node* child = m_lastChild; child; child = child->m_previousSibling)
			child->m_nextSibling.reset();
	}

	m_firstChild = nullptr;
	m_lastChild = nullptr;
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
	for (Node* child = m_firstChild; child != nullptr; child = child->m_nextSibling)
		child->write(os);
}

void Node::unlink()
{
	if (m_parent)
	{
		m_parent->removeChild(this);
		m_parent = nullptr;
	}
}

void Node::addChild(Node* child)
{
	child->unlink();

	T_ASSERT(child->m_previousSibling == nullptr);
	T_ASSERT(child->m_nextSibling == nullptr);

	child->m_parent = this;
	child->m_previousSibling = m_lastChild;

	if (m_lastChild)
		m_lastChild->m_nextSibling = child;
	else
		m_firstChild = child;

	m_lastChild = child;
}

void Node::removeChild(Node* child)
{
	T_ASSERT(child->m_parent == this);

	if (child->m_previousSibling)
		child->m_previousSibling->m_nextSibling = child->m_nextSibling;

	if (child->m_nextSibling)
		child->m_nextSibling->m_previousSibling = child->m_previousSibling;

	if (m_firstChild == child)
		m_firstChild = child->m_nextSibling;
	if (m_lastChild == child)
		m_lastChild = child->m_previousSibling;

	child->m_parent = nullptr;
	child->m_previousSibling = nullptr;
	child->m_nextSibling = nullptr;
}

void Node::removeAllChildren()
{
	for (Ref< Node > child = m_firstChild; child; child = child->m_nextSibling)
	{
		T_ASSERT(child->m_parent == this);
		child->m_parent = nullptr;
		child->m_previousSibling = nullptr;
	}
	m_firstChild = nullptr;
	m_lastChild = nullptr;
}

void Node::insertBefore(Node* child, Node* beforeNode)
{
	T_ASSERT(child->m_parent == nullptr);
	T_ASSERT(child->m_previousSibling == nullptr);
	T_ASSERT(!beforeNode || beforeNode->m_parent == this);

	if (!beforeNode)
		beforeNode = m_firstChild;

	child->m_parent = this;

	child->m_previousSibling = beforeNode ? beforeNode->m_previousSibling : nullptr;
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
	T_ASSERT(child->m_parent == nullptr);
	T_ASSERT(!afterNode || afterNode->m_parent == this);

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
		T_ASSERT(m_firstChild == nullptr);
		addChild(child);
	}
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

void Node::cloneChildren(Node* clone) const
{
	for (Node* child = m_firstChild; child; child = child->m_nextSibling)
	{
		Ref< Node > childClone = child->cloneUntyped();
		clone->addChild(childClone);
	}
}

}
