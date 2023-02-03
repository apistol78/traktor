/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <utility>
#include <vector>
#include "Ui/Layout.h"
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

/*! Flow layout.
 * \ingroup UI
 */
class T_DLLCLASS FlowLayout : public Layout
{
	T_RTTI_CLASS;

public:
	FlowLayout();

	explicit FlowLayout(int32_t marginX, int32_t marginY, int32_t padX, int32_t padY);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	Size m_margin;
	Size m_pad;

	bool calculateRects(Widget* widget, const Size& bounds, std::vector< WidgetRect >& outRects) const;
};

}
