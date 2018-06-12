#include "Core/Io/OutputStream.h"
#include "Core/Log/LogStreamTarget.h"

namespace traktor
{

LogStreamTarget::LogStreamTarget(OutputStream* stream)
:	m_stream(stream)
{
}

void LogStreamTarget::log(uint32_t threadId, int32_t level, const std::wstring& str)
{
	(*m_stream) << str << Endl;
}

}
