/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Node.h"

namespace traktor::html
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Node", Node, Object)

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
	child->m_nextSibling = nullptr;

	if (m_lastChild != nullptr)
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
