#include "Core/Io/IStream.h"
#include "Core/Misc/TString.h"
#include "Core/System/PipeReader.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PipeReader", PipeReader, Object)

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
	char pch = 0;

	Timer timer;
	timer.start();
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
			if (int32_t(1000.0f * timer.getElapsedTime()) > timeout)
				return false;

			ThreadManager::getInstance().getCurrentThread()->sleep(10);
		}
		
		// Extract character from stream.
		int32_t res = m_stream->read(&ch, sizeof(ch));
		if (res <= 0)
			return false;

#if defined(__APPLE__)
		if (ch == 10)
		{
			outLine = mbstows(std::string(m_acc.begin(), m_acc.end()));
			m_acc.resize(0);
			break;			
		}
		else
			m_acc.push_back(ch);
#else
		if (ch == 13)
		{
			outLine = mbstows(std::string(m_acc.begin(), m_acc.end()));
			m_acc.resize(0);
			break;
		}
		else if (ch != 10)
			m_acc.push_back(ch);
#endif

		pch = ch;
	}

	return true;
}

}
