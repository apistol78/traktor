/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_INotificationIcon_H
#define traktor_ui_INotificationIcon_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class IWidget;
class ISystemBitmap;

/*! \brief NotificationIcon interface.
 * \ingroup UI
 */
class INotificationIcon
{
public:
	virtual bool create(const std::wstring& text, ISystemBitmap* image) = 0;

	virtual void destroy() = 0;

	virtual void setImage(ISystemBitmap* image) = 0;
};

	}
}

#endif	// traktor_ui_INotificationIcon_H
