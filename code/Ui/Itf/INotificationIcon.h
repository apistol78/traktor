#ifndef traktor_ui_INotificationIcon_H
#define traktor_ui_INotificationIcon_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class IWidget;
class IBitmap;

/*! \brief NotificationIcon interface.
 * \ingroup UI
 */
class INotificationIcon
{
public:
	virtual bool create(const std::wstring& text, IBitmap* image) = 0;

	virtual void destroy() = 0;

	virtual void setImage(IBitmap* image) = 0;
};

	}
}

#endif	// traktor_ui_INotificationIcon_H
