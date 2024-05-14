/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Widget.h"

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

class IBitmap;
class ToolBarItem;
class ToolTip;
class ToolTipEvent;

/*! Tool bar control.
 * \ingroup UI
 */
class T_DLLCLASS ToolBar : public Widget
{
	T_RTTI_CLASS;

public:
	ToolBar();

	bool create(Widget* parent, uint32_t style = WsNone);

	virtual void destroy() override;

	uint32_t addImage(IBitmap* image);

	Size getImageSize() const;

	uint32_t addItem(ToolBarItem* item);

	void setItem(uint32_t id, ToolBarItem* item);

	Ref< ToolBarItem > getItem(uint32_t id);

	Ref< ToolBarItem > getItem(const Point& at);

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	Ref< ToolTip > m_toolTip;
	int32_t m_style;
	RefArray< IBitmap > m_images;
	RefArray< ToolBarItem > m_items;
	Ref< ToolBarItem > m_trackItem;
	int32_t m_offsetX;

	void clampOffset();

	void eventMouseTrack(MouseTrackEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventShowTip(ToolTipEvent* event);
};

	}
}

