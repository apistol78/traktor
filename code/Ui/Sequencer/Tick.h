/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Sequencer/Key.h"

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

/*! Sequencer tick.
 * \ingroup UI
 */
class T_DLLCLASS Tick : public Key
{
	T_RTTI_CLASS;

public:
	Tick(int time, bool movable);

	void setTime(int time);

	int getTime() const;

	virtual void move(int offset) override;

	virtual void getRect(SequencerControl* sequencer, const Sequence* sequence, const Rect& rcClient, Rect& outRect) const override;

	virtual void paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) override;

private:
	int m_time;
	bool m_movable;
};

	}
}

