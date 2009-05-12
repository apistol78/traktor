#include "Ui/Custom/Sequencer/Range.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Range", Range, Key)

Range::Range(int start, int end)
:	m_start(start)
,	m_end(end)
{
}

void Range::setStart(int start)
{
	m_start = start;
}

int Range::getStart() const
{
	return m_start;
}

void Range::setEnd(int end)
{
	m_end = end;
}

int Range::getEnd() const
{
	return m_end;
}

bool Range::hit(const ui::Point& position) const
{
	return false;
}

void Range::paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient)
{
	int x1 = sequence->clientFromTime(m_start);
	int x2 = sequence->clientFromTime(m_end);

	Rect rc(rcClient.left + x1, rcClient.top + 2, rcClient.left + x2, rcClient.bottom - 2);

	canvas.setForeground(Color(220, 255, 220));
	canvas.setBackground(Color(180, 230, 180));
	canvas.fillGradientRect(rc);

	canvas.setForeground(Color(0, 0, 0, 128));
	canvas.drawRect(rc);
}

		}
	}
}
