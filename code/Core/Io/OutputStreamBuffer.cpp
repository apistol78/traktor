#include "Core/Io/OutputStreamBuffer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OutputStreamBuffer", OutputStreamBuffer, IOutputStreamBuffer)

OutputStreamBuffer::OutputStreamBuffer()
:	m_indent(0)
,	m_decimals(6)
,	m_pushIndent(false)
{
}

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
