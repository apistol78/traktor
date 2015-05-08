#ifndef traktor_ui_custom_BackgroundWorkerDialog_H
#define traktor_ui_custom_BackgroundWorkerDialog_H

#include "Ui/Dialog.h"
#include "Core/Functor/Functor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

	struct IWorkerStatus : public IRefCount
	{
		virtual bool read(int32_t& outStep, std::wstring& outStatus) = 0;
	};

	BackgroundWorkerDialog();

	bool create(Widget* parent, const std::wstring& title, const std::wstring& message, bool abortButton);

	bool execute(IWaitable* waitable, IWorkerStatus* status);

	bool execute(const std::vector< IWaitable* >& waitables, IWorkerStatus* status);

private:
	std::vector< IWaitable* > m_waitables;
	Ref< IWorkerStatus > m_status;
	Ref< Static > m_labelMessage;
	Ref< Static > m_labelStatus;
	Ref< ProgressBar > m_progressBar;
	Ref< Button > m_buttonAbort;

	void eventAbortClick(ButtonClickEvent* event);

	void eventTimer(TimerEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BackgroundWorkerDialog_H
