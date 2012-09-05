#include "Ui/Custom/Sequencer/Tick.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Tick", Tick, Key)

Tick::Tick(int time)
:	m_time(time)
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
	m_time += offset;
}

void Tick::getRange(const Sequence* sequence, int& outLeft, int& outRight) const
{
	int x = sequence->clientFromTime(m_time);
	outLeft = x - 3;
	outRight = x + 4;
}

void Tick::paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	int x = sequence->clientFromTime(m_time) - scrollOffset;

	Rect rc(rcClient.left + x - 3, rcClient.top + 2, rcClient.left + x + 4, rcClient.bottom - 3);

	if (sequence->getSelectedKey() != this)
	{
		canvas.setForeground(Color4ub(220, 220, 255));
		canvas.setBackground(Color4ub(180, 180, 230));
	}
	else
	{
		canvas.setForeground(Color4ub(180, 180, 255));
		canvas.setBackground(Color4ub(160, 160, 255));
	}

	canvas.fillGradientRect(rc);

	canvas.setForeground(Color4ub(0, 0, 0, 128));
	canvas.drawRect(rc);
}

		}
	}
}
