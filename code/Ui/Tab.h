/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;
class TabPage;

/*! Tab container.
 * \ingroup UI
 */
class T_DLLCLASS Tab : public Widget
{
	T_RTTI_CLASS;

public:
	enum
	{
		WsCloseButton = WsUser,
		WsLine = WsUser << 1,
		WsBottom = WsUser << 2,
		WsDefault = WsCloseButton
	};

	Tab();

	bool create(Widget* parent, int32_t style = WsDefault);

	virtual Rect getInnerRect() const override;

	int32_t addImage(IBitmap* image, int32_t imageCount);

	int32_t addPage(TabPage* page);

	int32_t getPageCount() const;

	TabPage* getPage(int32_t index) const;

	TabPage* getPageAt(const Point& position) const;

	bool removePage(TabPage* page);

	void removeAllPages();

	void setActivePage(TabPage* page);

	TabPage* getActivePage();

	TabPage* cycleActivePage(bool forward);

	void addSelChangeEventHandler(EventHandler* eventHandler);

	void addCloseEventHandler(EventHandler* eventHandler);

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

private:
	struct PageState
	{
		Ref< TabPage > page;
		int32_t right;
		int32_t depth;

		PageState(TabPage* page = nullptr, int32_t right = 0);

		bool operator == (const PageState& rh) const;
	};

	Ref< IBitmap > m_bitmapClose;
	Ref< IBitmap > m_bitmapImages;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	Unit m_tabHeight;
	Rect m_innerRect;
	Font m_fontBold;
	AlignedVector< PageState > m_pages;
	Ref< TabPage > m_selectedPage;
	Ref< TabPage > m_hoverPage;
	bool m_closeButton;
	bool m_drawBorder;
	bool m_drawLine;
	bool m_bottom;

	PageState* findPageState(const TabPage* page);

	PageState* findPageState(int32_t depth);

	void eventMouseTrack(MouseTrackEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

}
