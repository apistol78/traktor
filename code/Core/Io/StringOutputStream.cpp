#include <cstring>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Align.h"

namespace traktor
{

StringOutputStreamBuffer::StringOutputStreamBuffer()
:	m_buffer((wchar_t*)getAllocator()->alloc(newCapacity * sizeof(wchar_t), 16, T_FILE_LINE))
,	m_capacity(1024)
,	m_tail(0)
{
	m_buffer[0] = L'\0';
}

bool StringOutputStreamBuffer::empty() const
{
	return m_tail == 0;
}

std::wstring StringOutputStreamBuffer::str() const
{
	return std::wstring(m_buffer.c_ptr());
}

const wchar_t* StringOutputStreamBuffer::c_str() const
{
	return m_buffer.c_ptr();
}

void StringOutputStreamBuffer::reset()
{
	m_tail = 0;
	m_buffer[0] = L'\0';
}

int32_t StringOutputStreamBuffer::overflow(const wchar_t* buffer, int32_t count)
{
	if (count <= 0)
		return 0;

	size_t newTail = m_tail + count;
	if (newTail + 1 >= m_capacity)
	{
		size_t newCapacity = alignDown(newTail, 1024) + 1024;

		// Allocate a new bigger buffer.
		AutoArrayPtr< wchar_t > newBuffer((wchar_t*)getAllocator()->alloc(newCapacity * sizeof(wchar_t), 16, T_FILE_LINE));
		std::memcpy(newBuffer.ptr(), m_buffer.c_ptr(), m_tail * sizeof(wchar_t));

		m_buffer.move(newBuffer);
		m_capacity = newCapacity;
	}

	// Concat at buffer tail.
	std::memcpy(m_buffer.ptr() + m_tail, buffer, count * sizeof(wchar_t));

	m_tail += count;
	m_buffer[m_tail] = L'\0';
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

	setBuffer(nullptr);

	T_EXCEPTION_GUARD_END
}

bool StringOutputStream::empty() const
{
	return m_buffer.empty();
}

std::wstring StringOutputStream::str() const
{
	return m_buffer.str();
}

const wchar_t* StringOutputStream::c_str() const
{
	return m_buffer.c_str();
}

void StringOutputStream::reset()
{
	m_buffer.reset();
}

}
