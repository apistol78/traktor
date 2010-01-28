#ifndef traktor_ui_custom_BackgroundWorkerStatus_H
#define traktor_ui_custom_BackgroundWorkerStatus_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS BackgroundWorkerStatus : public RefCountImpl< BackgroundWorkerDialog::IWorkerStatus >
{
public:
	BackgroundWorkerStatus(int32_t steps);

	void notify(int32_t step, const std::wstring& status);

	virtual bool read(int32_t& outStep, std::wstring& outStatus);

private:
	struct Notification
	{
		int32_t step;
		std::wstring status;
	};

	int32_t m_steps;
	Semaphore m_lock;
	std::list< Notification > m_notifications;
};

		}
	}
}

#endif	// traktor_ui_custom_BackgroundWorkerStatus_H
