#pragma once

#include "Core/Ref.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace sound
	{
	
class ISoundBuffer;
class ISoundBufferCursor;

class WaveformControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void setBuffer(const ISoundBuffer* buffer);

private:
	Ref< const ISoundBuffer > m_buffer;
	Ref< ISoundBufferCursor > m_cursor;

	void eventPaint(ui::PaintEvent* event);
};

	}
}
