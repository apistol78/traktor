/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

class DockPane;
class Image;
class ToolForm;

/*! Docking widget.
 * \ingroup UI
 */
class T_DLLCLASS Dock : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	virtual void destroy() override;

	DockPane* getPane() const;

	void showWidget(Widget* widget);

	void hideWidget(Widget* widget);

	bool isWidgetVisible(const Widget* widget) const;

	virtual void update(const Rect* rc = nullptr, bool immediate = false) override;

private:
	Ref< DockPane > m_pane;
	Ref< DockPane > m_splittingPane;
	Ref< DockPane > m_hintDockPane;
	Ref< ToolForm > m_hintDockForm;
	Ref< ToolForm > m_hint;
	Ref< Image > m_hintLeft;
	Ref< Image > m_hintRight;
	Ref< Image > m_hintTop;
	Ref< Image > m_hintBottom;
	EventSubject* m_ncButtonDownSender = nullptr;

	void eventSize(SizeEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventPaint(PaintEvent* event);

	void eventFormMove(MoveEvent* event);

	void eventFormNcButtonDown(NcMouseButtonDownEvent* event);

	void eventFormNcButtonUp(NcMouseButtonUpEvent* event);

	void eventHintButtonUp(MouseButtonUpEvent* event);
};

}
