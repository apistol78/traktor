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

class Layout;
class SizeEvent;

/*! Layout container.
 * \ingroup UI
 */
class T_DLLCLASS Container : public Widget
{
	T_RTTI_CLASS;

public:
	enum FitAxis
	{
		Horizontal = 1,
		Vertical = 2,
		Both = (Horizontal | Vertical)
	};

	bool create(Widget* parent, int style = WsNone, Layout* layout = nullptr);

	virtual void fit(uint32_t axis);

	virtual void update(const Rect* rc = nullptr, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	Ref< Layout > getLayout() const;

	void setLayout(Layout* layout);

private:
	Ref< Layout > m_layout;

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

}
