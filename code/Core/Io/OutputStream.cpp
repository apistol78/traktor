#include <sstream>
#include "Core/Io/OutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"

namespace traktor
{
	namespace
	{

const size_t c_flushInternalBufferSize = 16;

template < typename T, int size, int base >
wchar_t* uitoa__(T value, wchar_t* buf)
{
	const wchar_t digits[] = { L"0123456789ABCDEFG" };
	wchar_t* p = &buf[size - 1];
	*p-- = L'\0';

	if (value)
	{
		do
		{
			*p-- = digits[value % base];
			value /= base;
		}
		while (value);
	}
	else
		*p-- = digits[0];

	return p + 1;
}

template < typename T, typename UT, int size >
wchar_t* itoa__(T value, wchar_t* buf)
{
	bool negative = value < 0;
	UT un = negative ? UT(-value) : UT(value);

	wchar_t* p = &buf[size - 1];
	*p-- = L'\0';

	if (un)
	{
		do
		{
			*p-- = L'0' + uint8_t(un % 10);
			un /= 10;
		}
		while (un);
	}
	else
		*p-- = L'0';

	if (negative)
		*p-- = L'-';

	return p + 1;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.OutputStream", OutputStream, Object)

OutputStream::OutputStream(OutputStreamBuffer* buffer)
:	m_buffer(buffer)
,	m_pushIndent(false)
{
}

OutputStream::~OutputStream()
{
	T_EXCEPTION_GUARD_BEGIN

	flush();

	T_EXCEPTION_GUARD_END
}

void OutputStream::setBuffer(OutputStreamBuffer* buffer)
{
	m_buffer = buffer;
}

OutputStreamBuffer* OutputStream::getBuffer() const
{
	return m_buffer;
}

OutputStream& OutputStream::operator << (manipulator_t m)
{
	return m(*this);
}

OutputStream& OutputStream::operator << (const void* p)
{
	wchar_t buf[17];
	puts(uitoa__< size_t, sizeof_array(buf), 16 >(size_t(p), buf));
	return *this;
}

OutputStream& OutputStream::operator << (int8_t n)
{
	put(n);
	return *this;
}

OutputStream& OutputStream::operator << (uint8_t n)
{
	put(n);
	return *this;
}

OutputStream& OutputStream::operator << (int16_t n)
{
	wchar_t buf[7];
	puts(itoa__< int16_t, uint16_t, sizeof_array(buf) >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (uint16_t n)
{
	wchar_t buf[6];
	puts(uitoa__< uint16_t, sizeof_array(buf), 10 >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (int32_t n)
{
	wchar_t buf[12];
	puts(itoa__< int32_t, uint32_t, sizeof_array(buf) >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (uint32_t n)
{
	wchar_t buf[11];
	puts(uitoa__< uint32_t, sizeof_array(buf), 10 >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (int64_t n)
{
	wchar_t buf[64];
	puts(itoa__< int64_t, uint64_t, sizeof_array(buf) >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (uint64_t n)
{
	wchar_t buf[64];
	puts(uitoa__< uint64_t, sizeof_array(buf), 10 >(n, buf));
	return *this;
}

OutputStream& OutputStream::operator << (float f)
{
	std::wstringstream ss; ss << f;
	puts(ss.str().c_str());
	return *this;
}

OutputStream& OutputStream::operator << (double f)
{
	std::wstringstream ss; ss << f;
	puts(ss.str().c_str());
	return *this;
}

OutputStream& OutputStream::operator << (long double f)
{
	std::wstringstream ss; ss << f;
	puts(ss.str().c_str());
	return *this;
}

OutputStream& OutputStream::operator << (const wchar_t* s)
{
	puts(s);
	return *this;
}

OutputStream& OutputStream::operator << (const std::wstring& s)
{
	puts(s.c_str());
	return *this;
}

void OutputStream::put(wchar_t ch)
{
	Acquire< Semaphore > lock(m_lock);

	if (m_pushIndent && !m_indent.empty())
	{
		m_pushIndent = false;
		m_internal.insert(m_internal.end(), m_indent.begin(), m_indent.end());
	}

	m_internal.push_back(ch);
	if (m_internal.size() >= c_flushInternalBufferSize || ch == L'\n')
		flush();

	if (ch == L'\n')
		m_pushIndent = true;
}

void OutputStream::puts(const wchar_t* s)
{
	if (s)
	{
		Acquire< Semaphore > lock(m_lock);
		while (*s)
			put(*s++);
	}
}

void OutputStream::flush()
{
	Acquire< Semaphore > lock(m_lock);
	if (!m_internal.empty())
	{
		if (m_buffer)
			m_buffer->overflow(&m_internal[0], uint32_t(m_internal.size()));
		m_internal.resize(0);
	}
}

int OutputStream::getIndent() const
{
	return int(m_indent.size());
}

void OutputStream::setIndent(int indentCount)
{
	Acquire< Semaphore > lock(m_lock);
	m_indent = std::vector< wchar_t >(indentCount, L'\t');
}

void OutputStream::increaseIndent()
{
	Acquire< Semaphore > lock(m_lock);
	m_indent.push_back(L'\t');
}

void OutputStream::decreaseIndent()
{
	Acquire< Semaphore > lock(m_lock);
	m_indent.pop_back();
}

OutputStream& operator >> (OutputStream& os, wchar_t ch)
{
	os.put(ch);
	return os;
}

OutputStream& Endl(OutputStream& s)
{
#if defined(_WIN32) || defined(__APPLE__)
	s.puts(L"\r\n");
#else
	s.put(L'\n');
#endif
	return s;
}

OutputStream& IncreaseIndent(OutputStream& s)
{
	s.increaseIndent();
	return s;
}

OutputStream& DecreaseIndent(OutputStream& s)
{
	s.decreaseIndent();
	return s;
}

}
