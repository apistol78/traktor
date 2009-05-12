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

bool Tick::hit(const ui::Point& position) const
{
	return false;
}

void Tick::paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient)
{
	int x = sequence->clientFromTime(m_time);

	Rect rc(rcClient.left + x - 3, rcClient.top + 2, rcClient.left + x + 3, rcClient.bottom - 2);

	canvas.setForeground(Color(220, 255, 220));
	canvas.setBackground(Color(180, 230, 180));
	canvas.fillGradientRect(rc);

	canvas.setForeground(Color(0, 0, 0, 128));
	canvas.drawRect(rc);
}

		}
	}
}
