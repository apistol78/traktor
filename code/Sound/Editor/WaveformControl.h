/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{

class Bitmap;

	}

	namespace sound
	{

class ISoundBuffer;

class WaveformControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void setBuffer(const ISoundBuffer* buffer);

private:
	Ref< const ISoundBuffer > m_buffer;
	Ref< ui::Bitmap > m_waveform;

	void render(int32_t width, int32_t height, float start);

	void eventPaint(ui::PaintEvent* event);
};

	}
}
