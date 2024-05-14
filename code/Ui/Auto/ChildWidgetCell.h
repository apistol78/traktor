/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_ui_ChildWidgetCell_H
#define traktor_ui_ChildWidgetCell_H

#include "Ui/Widget.h"
#include "Ui/Auto/AutoWidgetCell.h"

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

/*! Auto widget cell.
 * \ingroup UI
 */
class T_DLLCLASS ChildWidgetCell : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	explicit ChildWidgetCell(Widget* child);

	virtual ~ChildWidgetCell();

	virtual void placeCells(AutoWidget* widget, const Rect& rect) override;

	Widget* getChild() const;

private:
	Ref< Widget > m_child;
};

	}
}

#endif	// traktor_ui_ChildWidgetCell_H

