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

namespace traktor
{
	namespace ui
	{

/*! Status bar control.
 * \ingroup UI
 */
class T_DLLCLASS StatusBar : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style = WsNone);

	void setAlert(bool alert);

	void addColumn(int32_t width);

	void setText(int32_t column, const std::wstring& text);

	virtual Size getPreferredSize(const Size& hint) const override;

private:
	struct Column
	{
		int32_t width;
		std::wstring text;
	};

	AlignedVector< Column > m_columns;
	bool m_alert = false;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

