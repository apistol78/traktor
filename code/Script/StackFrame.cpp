/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Script/StackFrame.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.StackFrame", 0, StackFrame, ISerializable)

void StackFrame::setFileName(const std::wstring& fileName)
{
	m_fileName = fileName;
}

const std::wstring& StackFrame::getFileName() const
{
	return m_fileName;
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
	s >> Member< std::wstring >(L"fileName", m_fileName);
	s >> Member< std::wstring >(L"functionName", m_functionName);
	s >> Member< uint32_t >(L"line", m_line);
}

}
