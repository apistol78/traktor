#ifndef traktor_ui_custom_BackgroundWorkerStatus_H
#define traktor_ui_custom_BackgroundWorkerStatus_H

#include <list>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Background worker status report.
 * \ingroup UIC
 */
class T_DLLCLASS BackgroundWorkerStatus : public Object
{
	T_RTTI_CLASS;

public:
	struct Notification
	{
		uint32_t step;
		std::wstring status;
	};

	BackgroundWorkerStatus(uint32_t steps);

	virtual void notify(uint32_t step, const std::wstring& status);

	virtual bool readNotification(Notification& outNotification);

	virtual uint32_t getSteps() const;

private:
	uint32_t m_steps;
	Semaphore m_lock;
	std::list< Notification > m_notifications;
};

		}
	}
}

#endif	// traktor_ui_custom_BackgroundWorkerStatus_H
