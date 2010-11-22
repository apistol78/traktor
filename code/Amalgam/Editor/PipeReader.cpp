#include "Amalgam/Editor/PipeReader.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PipeReader", PipeReader, Object)

PipeReader::PipeReader(IStream* stream)
:	m_stream(stream)
{
}

bool PipeReader::readLine(std::wstring& outLine, int32_t timeout)
{
	outLine.clear();

	if (!m_stream)
		return false;

	int32_t navail;
	char ch;

	for (;;)
	{
		for (;;)
		{
			if ((navail = m_stream->available()) < 0)
			{
				m_stream = 0;
				return false;
			}

			// Does the stream have a character ready then don't wait no more.
			if (navail > 0)
				break;

			// No character available; wait for more.
			if (timeout > 0)
			{
				ThreadManager::getInstance().getCurrentThread()->sleep(100);
				timeout -= 100;
			}
			else
				// No more time to wait; no line read.
				return false;
		}
		
		// Extract character from stream.
		int32_t res = m_stream->read(&ch, sizeof(ch));
		if (res <= 0)
			return false;

		if (ch == 13 || ch == 10)
		{
			outLine = mbstows(std::string(m_acc.begin(), m_acc.end()));
			m_acc.resize(0);
			break;
		}
		else
			m_acc.push_back(ch);
	}

	return true;
}

	}
}
