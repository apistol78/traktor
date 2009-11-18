#ifndef traktor_ui_NotificationIcon_H
#define traktor_ui_NotificationIcon_H

#include "Ui/EventSubject.h"
#include "Ui/EventHandler.h"
#include "Ui/Associative.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;
class Bitmap;
class INotificationIcon;

/*! \brief Notification icon.
 * \ingroup UI
 */
class T_DLLCLASS NotificationIcon
:	public EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	NotificationIcon();

	virtual ~NotificationIcon();

	bool create(const std::wstring& text, Bitmap* image);

	void destroy();

	/*! \brief Mouse button down event.
	*
	* Issued when mouse button has been pressed inside the widget.
	* Invoked with a MouseEvent.
	*/
	void addButtonDownEventHandler(EventHandler* eventHandler);

	/*! \brief Mouse button up event.
	*
	* Issued when mouse button has been released inside the widget.
	* Invoked with a MouseEvent.
	*/
	void addButtonUpEventHandler(EventHandler* eventHandler);

	/*! \brief Mouse button double click event.
	*
	* Issued when mouse button has been double clicked inside the widget.
	* Invoked with a MouseEvent.
	*/
	void addDoubleClickEventHandler(EventHandler* eventHandler);

private:
	INotificationIcon* m_ni;
};

	}
}

#endif	// traktor_ui_NotificationIcon_H
