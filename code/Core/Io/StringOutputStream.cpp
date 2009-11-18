#include "Core/Io/StringOutputStream.h"

namespace traktor
{

int StringOutputStreamBuffer::overflow(const wchar_t* buffer, int count)
{
	m_internal.insert(m_internal.end(), &buffer[0], &buffer[count]);
	return count;
}

StringOutputStream::StringOutputStream()
{
	setBuffer(&m_buffer);
}

StringOutputStream::~StringOutputStream()
{
	T_EXCEPTION_GUARD_BEGIN

	flush();
	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

bool StringOutputStream::empty()
{
	flush();
	return m_buffer.m_internal.empty();
}

std::wstring StringOutputStream::str()
{
	flush();

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
