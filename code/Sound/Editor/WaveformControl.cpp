#include "Core/Containers/AlignedVector.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/SoundMixer.h"
#include "Sound/Editor/WaveformControl.h"
#include "Ui/Application.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.WaveformControl", WaveformControl, Widget)

bool WaveformControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated))
		return false;

	addEventHandler< ui::PaintEvent >(this, &WaveformControl::eventPaint);

	return true;
}

void WaveformControl::setBuffer(const ISoundBuffer* buffer)
{
	m_buffer = buffer;
}

void WaveformControl::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect rcInner = getInnerRect();

	canvas.setBackground(Color4ub(40, 40, 40, 255));
	canvas.fillRect(rcInner);


	if (m_buffer)
	{
		m_cursor = m_buffer->createCursor();
		if (m_cursor)
		{
			SoundMixer mixer;

			float pixelsPerSecond = ui::dpi96(1000) / 1.0f;

			int32_t width = rcInner.getWidth();	// pixel(s)
			int32_t height = rcInner.getHeight();
		
			struct Span
			{
				float mn;
				float mx;

				Span()
				:	mn( std::numeric_limits< float >::max())
				,	mx(-std::numeric_limits< float >::max())
				{
				}

				bool empty() const
				{
					return mn > mx;
				}
			};

			AlignedVector< Span > spans;
			spans.resize(width);
		
			float time = 0.0f;
			bool going = true;
			while (going)
			{
				SoundBlock block = { { 0 }, 1024, 0, 0 };
				if (!m_buffer->getBlock(m_cursor, &mixer, block))
					break;

				float dT = 1.0f / block.sampleRate;

				for (int32_t i = 0; i < block.samplesCount; ++i)
				{
					float v = block.samples[0][i];

					int32_t x = int32_t(time * pixelsPerSecond);
					if (x < width)
					{
						Span& s = spans[x];
						if (s.empty())
						{
							s.mn = s.mx = v;
						}
						else
						{
							s.mn = std::min(s.mn, v);
							s.mx = std::max(s.mx, v);
						}
					}
					else
					{
						going = false;
						break;
					}

					time += dT;
				}
			}

			float v = 0.0f;
			for (int32_t i = 0; i < spans.size(); ++i)
			{
				Span& s = spans[i];
				if (s.empty())
					s.mn = s.mx = v;
				else
					v = s.mn;
			}

			canvas.setForeground(Color4ub(255, 255, 255, 255));
			for (int32_t i = 0; i < spans.size(); ++i)
			{
				Span& s = spans[i];
				canvas.drawLine(
					i,
					int32_t((-s.mn * 0.5f + 0.5f) * height),
					i,
					int32_t((-s.mx * 0.5f + 0.5f) * height)
				);
			}
		}
	}

	event->consume();
}

	}
}
