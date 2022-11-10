/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStreamBuffer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OutputStreamBuffer", OutputStreamBuffer, IOutputStreamBuffer)

int32_t OutputStreamBuffer::getIndent() const
{
	return m_indent;
}

void OutputStreamBuffer::setIndent(int32_t indent)
{
	m_indent = indent;
}

int32_t OutputStreamBuffer::getDecimals() const
{
	return m_decimals;
}

void OutputStreamBuffer::setDecimals(int32_t decimals)
{
	m_decimals = decimals;
}

bool OutputStreamBuffer::getPushIndent() const
{
	return m_pushIndent;
}

void OutputStreamBuffer::setPushIndent(bool pushIndent)
{
	m_pushIndent = pushIndent;
}

}
