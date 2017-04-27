/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/StackFrame.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.StackFrame", 0, StackFrame, ISerializable)

StackFrame::StackFrame()
:	m_line(0)
{
}

void StackFrame::setScriptId(const Guid& scriptId)
{
	m_scriptId = scriptId;
}

const Guid& StackFrame::getScriptId() const
{
	return m_scriptId;
}

void StackFrame::setFunctionName(const std::wstring& functionName)
{
	m_functionName = functionName;
}

const std::wstring& StackFrame::getFunctionName() const
{
	return m_functionName;
}

void StackFrame::setLine(uint32_t line)
{
	m_line = line;
}

uint32_t StackFrame::getLine() const
{
	return m_line;
}

void StackFrame::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scriptId", m_scriptId);
	s >> Member< std::wstring >(L"functionName", m_functionName);
	s >> Member< uint32_t >(L"line", m_line);
}

	}
}
