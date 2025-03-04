/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Canvas;
class IBitmap;
class MouseButtonDownEvent;
class MouseButtonUpEvent;
class MouseTrackEvent;
class ToolBar;

/*! Tool bar item.
 * \ingroup UI
 */
class T_DLLCLASS ToolBarItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	void setEnable(bool enable);

	bool isEnable() const;

protected:
	friend class ToolBar;

	virtual bool getToolTip(std::wstring& outToolTip) const = 0;

	virtual Size getSize(const ToolBar* toolBar) const = 0;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images) = 0;

	/*! Mouse enter item.
	 *
	 * \return True if tracking of item desired; false will not cause mouse to be captured.
	 */
	virtual bool mouseEnter(ToolBar* toolBar) = 0;

	virtual void mouseLeave(ToolBar* toolBar) = 0;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) = 0;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) = 0;

private:
	bool m_enable = true;
};

}
