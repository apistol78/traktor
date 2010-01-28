#include "Core/Thread/Acquire.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

BackgroundWorkerStatus::BackgroundWorkerStatus(int32_t steps)
:	m_steps(steps)
{
}

void BackgroundWorkerStatus::notify(int32_t step, const std::wstring& status)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Notification notification = { step, status };
	m_notifications.push_back(notification);
}

bool BackgroundWorkerStatus::read(int32_t& outStep, std::wstring& outStatus)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_notifications.empty())
		return false;

	outStep = m_steps > 0 ? int32_t(100 * m_notifications.front().step) / m_steps : -1;
	outStatus = m_notifications.front().status;
	m_notifications.pop_front();

	return true;
}

		}
	}
}
