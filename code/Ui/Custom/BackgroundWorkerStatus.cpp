#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.BackgroundWorkerStatus", BackgroundWorkerStatus, Object)

BackgroundWorkerStatus::BackgroundWorkerStatus(uint32_t steps)
:	m_steps(steps)
{
}

void BackgroundWorkerStatus::notify(uint32_t step, const std::wstring& status)
{
	Acquire< Semaphore > scope(m_lock);

	Notification notification = { step, status };
	m_notifications.push_back(notification);
}

bool BackgroundWorkerStatus::readNotification(Notification& outNotification)
{
	Acquire< Semaphore > scope(m_lock);

	if (m_notifications.empty())
		return false;

	outNotification = m_notifications.front();
	m_notifications.pop_front();

	return true;
}

uint32_t BackgroundWorkerStatus::getSteps() const
{
	return m_steps;
}

		}
	}
}
