/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/Common/Array.h"
#include "Flash/Action/Common/XMLNode.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.XMLNode", XMLNode, ActionObjectRelay)

XMLNode::XMLNode(
	const char* const prototype,
	NodeType nodeType,
	const std::wstring& localName,
	const std::wstring& nodeValue
)
:	ActionObjectRelay(prototype)
,	m_nodeType(nodeType)
,	m_localName(localName)
,	m_nodeValue(nodeValue)
,	m_parent(0)
,	m_previousSibling(0)
{
}

XMLNode::NodeType XMLNode::getNodeType() const
{
	return m_nodeType;
}

const std::wstring& XMLNode::getLocalName() const
{
	return m_localName;
}

const std::wstring& XMLNode::getNodeName() const
{
	return m_nodeName;
}

const std::wstring& XMLNode::getNodeValue() const
{
	return m_nodeValue;
}

const std::wstring& XMLNode::getNamespaceURI() const
{
	return m_namespaceURI;
}

const std::wstring& XMLNode::getPrefix() const
{
	return m_prefix;
}

void XMLNode::appendChild(XMLNode* node)
{
	T_ASSERT (node->m_parent == 0);

	node->m_parent = this;

	if (m_lastChild)
	{
		node->m_previousSibling = m_lastChild;
		m_lastChild->m_nextSibling = node;
		m_lastChild = node;
	}
	else
	{
		m_firstChild =
		m_lastChild = node;
	}
}

void XMLNode::insertBefore(XMLNode* node, const XMLNode* before)
{
}

void XMLNode::removeNode()
{
}

Ref< XMLNode > XMLNode::cloneNode() const
{
	return 0;
}

XMLNode* XMLNode::getFirstChild() const
{
	return m_firstChild;
}

XMLNode* XMLNode::getLastChild() const
{
	return m_lastChild;
}

XMLNode* XMLNode::getParentNode() const
{
	return m_parent;
}

XMLNode* XMLNode::getNextSibling() const
{
	return m_nextSibling;
}

XMLNode* XMLNode::getPreviousSibling() const
{
	return m_previousSibling;
}

void XMLNode::trace(visitor_t visitor) const
{
	ActionObjectRelay::trace(visitor);

	visitor(m_firstChild);
	visitor(m_lastChild);
	visitor(m_nextSibling);
}

void XMLNode::dereference()
{
	ActionObjectRelay::dereference();

	m_firstChild = 0;
	m_lastChild = 0;
	m_nextSibling = 0;
}

	}
}
