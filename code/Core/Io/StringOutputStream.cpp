#include "Core/Io/StringOutputStream.h"

namespace traktor
{

StringOutputStreamBuffer::StringOutputStreamBuffer()
{
	m_internal.reserve(1024);
}

bool StringOutputStreamBuffer::empty() const
{
	return m_internal.empty();
}

std::wstring StringOutputStreamBuffer::str()
{
	if (m_internal.empty())
		return std::wstring();

	return std::wstring(
		m_internal.begin(),
		m_internal.end()
	);
}

void StringOutputStreamBuffer::reset()
{
	m_internal.resize(0);
}

int32_t StringOutputStreamBuffer::overflow(const wchar_t* buffer, int32_t count)
{
	m_internal.insert(m_internal.end(), &buffer[0], &buffer[count]);
	return count;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.StringOutputStream", StringOutputStream, OutputStream);

StringOutputStream::StringOutputStream()
{
	setBuffer(&m_buffer);
}

StringOutputStream::~StringOutputStream()
{
	T_EXCEPTION_GUARD_BEGIN

	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

bool StringOutputStream::empty() const
{
	return m_buffer.empty();
}

std::wstring StringOutputStream::str()
{
	return m_buffer.str();
}

void StringOutputStream::reset()
{
	m_buffer.reset();
}

}
