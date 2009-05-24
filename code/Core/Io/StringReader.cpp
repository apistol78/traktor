#include "Core/Io/StringReader.h"
#include "Core/Io/Stream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.StringReader", StringReader, Object)

StringReader::StringReader(Stream* stream, Encoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_count(0)
{
}

int StringReader::readLine(std::wstring& out)
{
	wchar_t ch;

	out = L"";

	for (;;)
	{
		if (m_count < sizeof(m_buffer))
		{
			int result = m_stream->read(&m_buffer[m_count], sizeof(m_buffer) - m_count);
			if (result > 0)
				m_count += result;
			else if (out.empty())
				return -1;
		}
		
		if (m_count <= 0)
			break;

		int result = m_encoding->translate(m_buffer, m_count, ch);
		if (result <= 0)
			break;

		std::memmove(&m_buffer[0], &m_buffer[result], m_count - result);
		m_count -= result;

		if (ch == L'\n')
			break;
		else if (ch != L'\r')
			out += ch;
	}

	return int(out.length());
}

}
