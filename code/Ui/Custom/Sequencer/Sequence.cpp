#include <algorithm>
#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Key.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sequenceHeight = 22;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Sequence", Sequence, SequenceItem)

Sequence::Sequence(const std::wstring& name)
:	SequenceItem(name)
,	m_previousPosition(0)
,	m_timeScale(8)
{
}

void Sequence::addKey(Key* key)
{
	m_keys.push_back(key);
}

void Sequence::removeKey(Key* key)
{
	m_keys.remove(key);
}

void Sequence::removeAllKeys()
{
	m_keys.resize(0);
}

bool Sequence::containsKey(Key* key) const
{
	return bool(std::find(m_keys.begin(), m_keys.end(), key) != m_keys.end());
}

const RefArray< Key >& Sequence::getKeys() const
{
	return m_keys;
}

Ref< Key > Sequence::getSelectedKey() const
{
	return m_selectedKey;
}

int Sequence::clientFromTime(int time) const
{
	return time / m_timeScale;
}

int Sequence::timeFromClient(int client) const
{
	return client * m_timeScale;
}

void Sequence::mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	if (at.x < separator)
		return;

	m_selectedKey = 0;
	m_trackKey = 0;

	for (RefArray< Key >::const_iterator j = m_keys.begin(); j != m_keys.end(); ++j)
	{
		int left, right;
		(*j)->getRange(this, left, right);

		left += separator - scrollOffset;
		right += separator - scrollOffset;

		if (at.x >= left && at.x <= right)
		{
			m_previousPosition = at.x;
			m_selectedKey = *j;
			m_trackKey = *j;
			break;
		}
	}
}

void Sequence::mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	m_previousPosition = 0;
	m_trackKey = 0;
}

void Sequence::mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	if (button && m_trackKey)
	{
		int32_t offset = timeFromClient(at.x - m_previousPosition);
		if (offset != 0)
		{
			m_trackKey->move(offset);

			CommandEvent cmdEvent(sequencer, m_trackKey, Command(offset));
			sequencer->raiseEvent(SequencerControl::EiKeyMove, &cmdEvent);
		}
		m_previousPosition = at.x;
	}
}

void Sequence::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
	// Save time scale here; it's used in client<->time conversion.
	m_timeScale = sequencer->getTimeScale();

	// Draw sequence background.
	if (!isSelected())
	{
		canvas.setForeground(Color(250, 249, 250));
		canvas.setBackground(Color(238, 237, 240));
		canvas.fillGradientRect(Rect(rc.left, rc.top, separator, rc.bottom));
		canvas.setForeground(Color(170, 169, 170));
		canvas.setBackground(Color(158, 157, 160));
		canvas.fillGradientRect(Rect(separator, rc.top, rc.right, rc.bottom));
	}
	else
	{
		canvas.setBackground(Color(226, 229, 238));
		canvas.fillRect(Rect(rc.left, rc.top, separator, rc.bottom));
		canvas.setBackground(Color(206, 209, 218));
		canvas.fillRect(Rect(separator, rc.top, rc.right, rc.bottom));
	}

	canvas.setForeground(Color(128, 128, 128));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);

	// Draw sequence text.
	canvas.setForeground(Color(0, 0, 0));
	Size ext = canvas.getTextExtent(getName());
	canvas.drawText(
		Point(
			rc.left + 32 + getDepth() * 16,
			rc.top + (rc.getHeight() - ext.cy) / 2
		),
		getName()
	);

	// Draw sequence keys.
	canvas.setClipRect(Rect(
		rc.left + separator,
		rc.top,
		rc.right,
		rc.bottom
	));

	// Draw tickers.
	canvas.setForeground(Color(128, 128, 128));
	int cy = (rc.top + rc.bottom) / 2;
	for (int i = 0; i < sequencer->getLength(); i += 100)
	{
		int cx = separator + clientFromTime(i) - scrollOffset;
		if (cx > rc.right)
			break;
		int cya = (i % 1000 == 0) ? 4 : 0;
		canvas.drawLine(cx, cy - 2 - cya, cx, cy + 1 + cya);
	}

	for (RefArray< Key >::const_iterator j = m_keys.begin(); j != m_keys.end(); ++j)
	{
		Rect rcClient(
			rc.left + separator,
			rc.top,
			rc.right,
			rc.bottom
		);
		(*j)->paint(canvas, this, rcClient, scrollOffset);
	}
}

		}
	}
}
