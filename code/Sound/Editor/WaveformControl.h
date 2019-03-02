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
