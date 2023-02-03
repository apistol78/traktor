/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Layout.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Center layout.
 * \ingroup UI
 */
class T_DLLCLASS CenterLayout : public Layout
{
	T_RTTI_CLASS;

public:
	enum CenterAxis
	{
		CaHorizontal = 1,
		CaVertical = 2,
		CaBoth = CaHorizontal | CaVertical
	};

	CenterLayout(uint32_t centerAxis = CaBoth);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	uint32_t m_centerAxis;
};

}
