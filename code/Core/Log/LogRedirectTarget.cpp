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

void LogRedirectTarget::log(uint32_t threadId, int32_t level, const std::wstring& str)
{
	for (RefArray< ILogTarget >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
		(*i)->log(threadId, level, str);
}

}
