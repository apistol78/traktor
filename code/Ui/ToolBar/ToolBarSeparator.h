/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ToolBar/ToolBarItem.h"

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

/*! Tool bar separator.
 * \ingroup UI
 */
class T_DLLCLASS ToolBarSeparator : public ToolBarItem
{
	T_RTTI_CLASS;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const override final;

	virtual Size getSize(const ToolBar* toolBar) const override final;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, const RefArray< IBitmap >& images) override final;

	virtual bool mouseEnter(ToolBar* toolBar) override final;

	virtual void mouseLeave(ToolBar* toolBar) override final;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) override final;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) override final;
};

	}
}

