#include "Core/Io/StringOutputStream.h"

namespace traktor
{

StringOutputStreamBuffer::StringOutputStreamBuffer()
{
	m_internal.reserve(1024);
}

int StringOutputStreamBuffer::overflow(const wchar_t* buffer, int count)
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

bool StringOutputStream::empty()
{
	return m_buffer.m_internal.empty();
}

std::wstring StringOutputStream::str()
{
	if (m_buffer.m_internal.empty())
		return std::wstring();

	return std::wstring(
		m_buffer.m_internal.begin(),
		m_buffer.m_internal.end()
	);
}

void StringOutputStream::reset()
{
	m_buffer.m_internal.resize(0);
}

}
