/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IPopupMenu_H
#define traktor_ui_IPopupMenu_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class MenuItem;
class Point;
class IWidget;

/*! \brief PopupMenu interface.
 * \ingroup UI
 */
class IPopupMenu
{
public:
	virtual bool create() = 0;

	virtual void destroy() = 0;

	virtual void add(MenuItem* item) = 0;

	virtual MenuItem* show(IWidget* parent, const Point& at) = 0;
};

	}
}

#endif	// traktor_ui_IPopupMenu_H
