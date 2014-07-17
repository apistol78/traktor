#ifndef traktor_ui_NotificationIcon_H
#define traktor_ui_NotificationIcon_H

#include "Ui/Associative.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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

	void setImage(Bitmap* image);

private:
	INotificationIcon* m_ni;
};

	}
}

#endif	// traktor_ui_NotificationIcon_H
