#include "Ui/Canvas.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Track.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Track", Track, Key)

Track::Track(int start, int end, bool movable)
:	m_start(start)
,	m_end(end)
,	m_movable(movable)
{
}

void Track::setStart(int start)
{
	m_start = start;
}

int Track::getStart() const
{
	return m_start;
}

void Track::setEnd(int end)
{
	m_end = end;
}

int Track::getEnd() const
{
	return m_end;
}

void Track::move(int offset)
{
	if (m_movable)
	{
		m_start += offset;
		m_end += offset;
	}
}

void Track::getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	outRect.left = sequence->clientFromTime(m_start);
	outRect.top = rcClient.top;
	outRect.right = sequence->clientFromTime(m_end);
	outRect.bottom = rcClient.bottom - 1;
}

void Track::paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	int x1 = sequence->clientFromTime(m_start) - scrollOffset;
	int x2 = sequence->clientFromTime(m_end) - scrollOffset;

	Rect rc(rcClient.left + x1, rcClient.top, rcClient.left + x2, rcClient.bottom - 1);

	canvas.setForeground(Color4ub(255, 255, 255, 100));
	canvas.setBackground(Color4ub(255, 255, 200, 100));
	canvas.fillGradientRect(rc);

	canvas.setForeground(Color4ub(255, 255, 200, 180));
	canvas.drawRect(rc);
}

		}
	}
}
