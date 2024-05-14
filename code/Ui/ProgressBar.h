/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
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

/*! Progress bar.
 * \ingroup UI
 */
class T_DLLCLASS ProgressBar : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style = WsNone, int32_t minProgress = 0, int32_t maxProgress = 100);

	void setRange(int32_t minProgress, int32_t maxProgress);

	int32_t getMinRange() const;

	int32_t getMaxRange() const;

	void setProgress(int32_t progress);

	int32_t getProgress() const;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	int32_t m_minProgress = 0;
	int32_t m_maxProgress = 100;
	int32_t m_progress = 0;
	int32_t m_loop = 0;

	void eventPaint(PaintEvent* event);

	void eventTimer(TimerEvent* event);
};

}
