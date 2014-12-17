#pragma optimize( "", off )

#include "Script/Editor/IScriptOutline.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.IScriptOutline", IScriptOutline, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.IScriptOutline.Node", IScriptOutline::Node, Object)

IScriptOutline::Node::Node(int32_t line)
:	m_line(line)
{
}

void IScriptOutline::Node::setNext(Node* node)
{
	m_next = node;
}

void IScriptOutline::Node::setNextTail(Node* node)
{
	if (m_next)
		m_next->setNextTail(node);
	else
		m_next = node;
}

IScriptOutline::Node* IScriptOutline::Node::getNext() const
{
	return m_next;
}

int32_t IScriptOutline::Node::getLine() const
{
	return m_line;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.IScriptOutline.FunctionReferenceNode", IScriptOutline::FunctionReferenceNode, IScriptOutline::Node)

IScriptOutline::FunctionReferenceNode::FunctionReferenceNode(int32_t line, const std::wstring& name)
:	Node(line)
,	m_name(name)
{
}

const std::wstring& IScriptOutline::FunctionReferenceNode::getName() const
{
	return m_name;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.IScriptOutline.FunctionNode", IScriptOutline::FunctionNode, IScriptOutline::Node)

IScriptOutline::FunctionNode::FunctionNode(int32_t line, const std::wstring& name, bool local, Node* body)
:	Node(line)
,	m_name(name)
,	m_local(local)
,	m_body(body)
{
}

const std::wstring& IScriptOutline::FunctionNode::getName() const
{
	return m_name;
}

bool IScriptOutline::FunctionNode::isLocal() const
{
	return m_local;
}

IScriptOutline::Node* IScriptOutline::FunctionNode::getBody() const
{
	return m_body;
}

	}
}
