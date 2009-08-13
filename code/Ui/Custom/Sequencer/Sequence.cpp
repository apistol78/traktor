#include <algorithm>
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Key.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sequenceHeight = 22;
const int c_timeScaleDenom = 8;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Sequence", Sequence, SequenceItem)

Sequence::Sequence(const std::wstring& name)
:	SequenceItem(name)
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

const RefList< Key >& Sequence::getKeys() const
{
	return m_keys;
}

Key* Sequence::getSelectedKey() const
{
	return m_selectedKey;
}

int Sequence::clientFromTime(int time) const
{
	return time / c_timeScaleDenom;
}

void Sequence::mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	if (at.x < separator)
		return;

	m_selectedKey = 0;
	for (RefList< Key >::const_iterator j = m_keys.begin(); j != m_keys.end(); ++j)
	{
		int left, right;
		(*j)->getRange(this, left, right);

		left += separator - scrollOffset;
		right += separator - scrollOffset;

		if (at.x >= left && at.x <= right)
		{
			m_selectedKey = *j;
			break;
		}
	}
}

void Sequence::mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
}

void Sequence::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
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

	for (RefList< Key >::const_iterator j = m_keys.begin(); j != m_keys.end(); ++j)
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
