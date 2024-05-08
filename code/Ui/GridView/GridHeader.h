/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidgetCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class GridColumn;

class T_DLLCLASS GridHeader : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	void setColumns(const RefArray< GridColumn >& columns);

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) override final;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) override final;

	virtual void mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position) override final;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) override final;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;

private:
	RefArray< GridColumn > m_columns;
	Ref< GridColumn > m_resizeColumn;
	int32_t m_resizeWidth;
	int32_t m_resizePosition;
};

}
