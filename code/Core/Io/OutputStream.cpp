#include <cmath>
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Io/OutputStream.h"
#include "Core/Thread/Acquire.h"

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

template < typename T > struct ftoa_int_type {};
template <> struct ftoa_int_type < float > { typedef int32_t int_t; };
template <> struct ftoa_int_type < double > { typedef int64_t int_t; };

template < typename T, int fractions, int size >
wchar_t* ftoa__(T value, wchar_t* buf)
{
	typedef typename ftoa_int_type< T >::int_t int_t;

	bool negative = value < 0;
	T un = negative ? -value : value;

	wchar_t* p = &buf[size - 1];
	*p-- = L'\0';

	int_t vi = int_t(un);
	int_t vf = int_t((un - vi) * powf(10, fractions + 1));

	if (vf)
	{
		if (vf % 10 >= 5)
			vf += 10;

		vf /= 10;

		int_t nf = fractions;

		while ((vf % 10) == 0)
		{
			vf /= 10;
			nf--;
		}

		do
		{
			*p-- = L'0' + int_t(vf % 10);
			vf /= 10;
			nf--;
		}
		while (vf);

		while (nf > 0)
		{
			*p-- = L'0';
			nf--;
		}

		*p-- = L'.';
	}

	if (vi)
	{
		do 
		{
			*p-- = L'0' + int_t(vi % 10);
			vi /= 10;
		}
		while (vi);
	}
	else
		*p-- = L'0';

	if (negative)
		*p-- = L'-';

	return p + 1;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.OutputStream", OutputStream, Object);

OutputStream::OutputStream(IOutputStreamBuffer* buffer, LineEnd lineEnd)
:	m_buffer(buffer)
,	m_lineEnd(lineEnd)
,	m_pushIndent(false)
{
	if (m_lineEnd == LeAuto)
	{
#if defined(_WIN32)
		m_lineEnd = LeWin;
#else
		m_lineEnd = LeUnix;
#endif
	}
}

OutputStream::~OutputStream()
{
	T_EXCEPTION_GUARD_BEGIN

	flush();

	T_EXCEPTION_GUARD_END
}

void OutputStream::setBuffer(IOutputStreamBuffer* buffer)
{
	m_buffer = buffer;
}

IOutputStreamBuffer* OutputStream::getBuffer() const
{
	return m_buffer;
}

OutputStream::LineEnd OutputStream::getLineEnd() const
{
	return m_lineEnd;
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
	wchar_t buf[128];
	puts(ftoa__< float, 5, sizeof_array(buf) >(f, buf));
	return *this;
}

OutputStream& OutputStream::operator << (double f)
{
	wchar_t buf[256];
	puts(ftoa__< double, 8, sizeof_array(buf) >(f, buf));
	return *this;
}

OutputStream& OutputStream::operator << (long double f)
{
	wchar_t buf[256];
	puts(ftoa__< double, 8, sizeof_array(buf) >(f, buf));
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

	bool eol = false;
	switch (m_lineEnd)
	{
	case LeWin:
	case LeUnix:
		eol = bool(ch == L'\n');
		break;

	case LeMac:
		eol = bool(ch == L'\r');
		break;
	}

	if (m_internal.size() >= c_flushInternalBufferSize || eol)
		flush();

	if (eol)
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
	if (!m_indent.empty())
		m_indent.pop_back();
}

OutputStream& operator << (OutputStream& os, wchar_t ch)
{
	os.put(ch);
	return os;
}

OutputStream& Endl(OutputStream& s)
{
	switch (s.getLineEnd())
	{
	case OutputStream::LeWin:
		s.puts(L"\r\n");
		break;

	case OutputStream::LeMac:
		s.put(L'\r');
		break;

	case OutputStream::LeUnix:
		s.put(L'\n');
		break;
	}
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

OutputStream& FormatMultipleLines(OutputStream& s, const std::wstring& str)
{
	size_t p0 = 0;
	for (uint32_t ln = 1;; ++ln)
	{
		size_t p1 = str.find('\n', p0);
		s << ln << L": " << str.substr(p0, p1 - p0 - 1) << Endl;
		if (p1 != str.npos)
			p0 = p1 + 1;
		else
			break;
	}
	return s;
}

}
