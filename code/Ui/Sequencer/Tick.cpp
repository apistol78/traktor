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

const int c_sequenceHeight = 40;

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
	int32_t sequenceHeight = dpi96(c_sequenceHeight);
	int32_t x = sequence->clientFromTime(m_time);
	outRect.left = x - 3;
	outRect.top = rcClient.top + 2;
	outRect.right = x + 4;
	outRect.bottom = rcClient.top + sequenceHeight - 3;
}

void Tick::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	int32_t sequenceHeight = dpi96(c_sequenceHeight);
	int32_t x = sequence->clientFromTime(m_time) - scrollOffset;
	Rect rc(rcClient.left + x - 3, rcClient.top + 2, rcClient.left + x + 4, rcClient.top + sequenceHeight - 3);
	bool selected = (sequence->getSelectedKey() == this);

	canvas.setBackground(ss->getColor(this, selected ? L"background-color-selected" : L"background-color"));
	canvas.fillRect(rc);

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawRect(rc);
}

	}
}
