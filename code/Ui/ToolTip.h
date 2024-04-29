/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::ui
{

/*! Tool tip control.
 * \ingroup UI
 */
class T_DLLCLASS ToolTip : public ToolForm
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	void show(const Point& at, const std::wstring& text);

private:
	bool m_tracking = false;
	uint32_t m_counter = 0;

	void eventTimer(TimerEvent* event);

	void eventPaint(PaintEvent* event);
};

}
