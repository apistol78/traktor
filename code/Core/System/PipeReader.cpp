#include <cstring>
#include "Core/Io/IStream.h"
#include "Core/Misc/TString.h"
#include "Core/System/PipeReader.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PipeReader", PipeReader, Object)
	
PipeReader::PipeReader(IStream* stream)
:	m_stream(stream)
{
}

PipeReader::~PipeReader()
{
}

PipeReader::Result PipeReader::readLine(std::wstring& outLine, int32_t timeout)
{
	char buffer[128];

	outLine.clear();

	// Pop line from line queue if any.
	if (!m_lines.empty())
	{
		outLine = m_lines.front();
		m_lines.pop_front();
		return RtOk;
	}

	// Cannot continue after stream has closed.
	if (!m_stream)
		return RtEnd;

	while (m_lines.empty())
	{
		std::memset(buffer, 0, sizeof(buffer));
		int32_t nrecv = m_stream->read(buffer, sizeof(buffer));
		if (nrecv < 0)
		{
			m_stream = 0;
			break;
		}

		if (nrecv == 0)
		{
			if (timeout <= 0)
				break;

			timeout -= 10;
			ThreadManager::getInstance().getCurrentThread()->sleep(10);
			continue;
		}

		// Transform into lines.
		for (int32_t i = 0; i < nrecv; ++i)
		{
			char ch = buffer[i];

#if defined(__APPLE__)
			if (ch == 10)
			{
				m_lines.push_back(mbstows(std::string(m_acc.begin(), m_acc.end())));
				m_acc.resize(0);
			}
			else
				m_acc.push_back(ch);
#else
			if (ch == 13)
			{
				m_lines.push_back(mbstows(std::string(m_acc.begin(), m_acc.end())));
				m_acc.resize(0);
			}
			else if (ch != 10)
				m_acc.push_back(ch);
#endif		
		}
	}

	if (m_lines.empty())
		return RtTimeout;

	// Pop line from queue.
	outLine = m_lines.front();
	m_lines.pop_front();
	return RtOk;
}

}
