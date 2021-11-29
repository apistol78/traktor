#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Tick.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_sequenceHeight = 25;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Tick", Tick, Key)

Tick::Tick(int time, bool movable)
:	m_time(time)
,	m_movable(movable)
{
}

void Tick::setTime(int time)
{
	m_time = time;
}

int Tick::getTime() const
{
	return m_time;
}

void Tick::move(int offset)
{
	if (m_movable)
		m_time += offset;
}

void Tick::getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	const int32_t sequenceHeight = dpi96(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time);
	const int32_t hw = dpi96(3);
	const int32_t hh = dpi96(2);

	outRect.left = x - hw;
	outRect.top = rcClient.top + hh;
	outRect.right = x + hw + 1;
	outRect.bottom = rcClient.top + sequenceHeight - hh - 1;
}

void Tick::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	const int32_t sequenceHeight = dpi96(c_sequenceHeight);
	const int32_t x = sequence->clientFromTime(m_time) - scrollOffset;
	const int32_t hw = dpi96(3);
	const int32_t hh = dpi96(2);

	Rect rc(
		rcClient.left + x - hw,
		rcClient.top + hh,
		rcClient.left + x + hw + 1,
		rcClient.top + sequenceHeight - hh - 1
	);

	const bool selected = (sequence->getSelectedKey() == this);

	canvas.setBackground(ss->getColor(this, selected ? L"background-color-selected" : L"background-color"));
	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.fillRect(rc);
	canvas.drawRect(rc);
}

	}
}
