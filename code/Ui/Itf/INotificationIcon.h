#ifndef traktor_ui_INotificationIcon_H
#define traktor_ui_INotificationIcon_H

#include "Core/Config.h"
#include "Core/Heap/Ref.h"

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

class IWidget;
class IBitmap;

/*! \brief NotificationIcon interface.
 * \ingroup UI
 */
class T_DLLCLASS INotificationIcon
{
public:
	virtual bool create(const std::wstring& text, IBitmap* image) = 0;

	virtual void destroy() = 0;
};

	}
}

#endif	// traktor_ui_INotificationIcon_H
