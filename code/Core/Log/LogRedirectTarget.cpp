#include "Core/Log/LogRedirectTarget.h"

namespace traktor
{

LogRedirectTarget::LogRedirectTarget(ILogTarget* target)
{
	m_targets.push_back(target);
}

LogRedirectTarget::LogRedirectTarget(ILogTarget* target1, ILogTarget* target2)
{
	m_targets.push_back(target1);
	m_targets.push_back(target2);
}

void LogRedirectTarget::log(uint32_t threadId, int32_t level, const wchar_t* str)
{
	for (const auto& target : m_targets)
		target->log(threadId, level, str);
}

}
