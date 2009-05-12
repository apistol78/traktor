#ifndef traktor_ui_custom_BackgroundWorkerDialog_H
#define traktor_ui_custom_BackgroundWorkerDialog_H

#include "Ui/Dialog.h"
#include "Core/Functor/Functor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace ui
	{

class Static;
class Button;

		namespace custom
		{

class ProgressBar;
class BackgroundWorkerStatus;

/*! \brief Background worker thread progress dialog.
 * \ingroup UIC
 */
class T_DLLCLASS BackgroundWorkerDialog : public Dialog
{
	T_RTTI_CLASS(BackgroundWorkerDialog)

public:
	enum StyleFlags
	{
		WsAbortButton = WsUser << 1
	};

	BackgroundWorkerDialog();

	bool create(Widget* parent, const std::wstring& title, const std::wstring& message, int style = WsDefaultFixed);

	bool execute(Thread* thread, BackgroundWorkerStatus* status);

	bool execute(const std::vector< Thread* >& thread, BackgroundWorkerStatus* status);

private:
	std::vector< Thread* > m_threads;
	Ref< BackgroundWorkerStatus > m_status;
	Ref< Static > m_labelMessage;
	Ref< Static > m_labelStatus;
	Ref< ProgressBar > m_progressBar;
	Ref< Button > m_buttonAbort;

	void eventAbortClick(Event* event);

	void eventTimer(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BackgroundWorkerDialog_H
