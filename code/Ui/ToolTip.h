/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ToolForm.h"

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

/*! Tool tip control.
 * \ingroup UI
 */
class T_DLLCLASS ToolTip : public ToolForm
{
	T_RTTI_CLASS;

public:
	ToolTip();

	bool create(Widget* parent);

	void show(const Point& at, const std::wstring& text);

private:
	bool m_tracking;
	uint32_t m_counter;

	void eventTimer(TimerEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

