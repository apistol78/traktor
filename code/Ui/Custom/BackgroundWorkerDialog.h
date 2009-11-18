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

class IWaitable;

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
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsAbortButton = WsUser << 1
	};

	BackgroundWorkerDialog();

	bool create(Widget* parent, const std::wstring& title, const std::wstring& message, int style = WsDefaultFixed);

	bool execute(IWaitable* waitable, BackgroundWorkerStatus* status);

	bool execute(const std::vector< IWaitable* >& waitables, BackgroundWorkerStatus* status);

private:
	std::vector< IWaitable* > m_waitables;
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
