#include "Core/Log/LogStreamTarget.h"
#include "Core/Misc/String.h"

namespace traktor
{

LogStreamTarget::LogStreamTarget(OutputStream* stream)
:	m_stream(stream)
{
}

void LogStreamTarget::log(uint32_t threadId, int32_t level, const wchar_t* str)
{
	(*m_stream) << traktor::str(L"[%5d] ", threadId) << str << Endl;
}

}
