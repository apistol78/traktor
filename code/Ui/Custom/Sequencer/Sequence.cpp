#include <algorithm>
#include "Ui/Canvas.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/SequenceButtonClickEvent.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Key.h"
#include "Ui/Custom/Sequencer/KeyMoveEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sequenceHeight = 22;
const int c_buttonSize = 18;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Sequence", Sequence, SequenceItem)

Sequence::Sequence(const std::wstring& name)
:	SequenceItem(name)
,	m_previousPosition(0)
,	m_timeScale(8)
{
}

int32_t Sequence::addButton(Bitmap* imageUp, Bitmap* imageDown, const Command& command)
{
	Button btn;
	btn.imageUp = imageUp;
	btn.imageDown = imageDown;
	btn.command = command;
	btn.state = false;
	m_buttons.push_back(btn);
	return int32_t(m_buttons.size() - 1);
}

bool Sequence::getButtonState(int32_t buttonIndex) const
{
	return m_buttons[buttonIndex].state;
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
	{
		for (int32_t i = 0; i < int32_t(m_buttons.size()); ++i)
		{
			if (m_buttons[i].rc.inside(Point(at.x + rc.left, at.y + rc.top)))
			{
				// Toggle button state.
				m_buttons[i].state = !m_buttons[i].state;

				// Notify button listeners.
				SequenceButtonClickEvent clickEvent(sequencer, this, m_buttons[i].command);
				sequencer->raiseEvent(&clickEvent);

				break;
			}
		}
	}
	else
	{
		m_selectedKey = 0;
		m_trackKey = 0;

		Rect rcClient(
			rc.left + separator,
			rc.top,
			rc.right,
			rc.bottom
		);

		for (size_t j = m_keys.size(); j > 0; --j)
		{
			Key* key = m_keys[j - 1];

			Rect rcKey;
			key->getRect(this, rcClient, rcKey);

			rcKey.left += separator - scrollOffset;
			rcKey.right += separator - scrollOffset;

			if (at.x >= rcKey.left && at.x <= rcKey.right)
			{
				m_previousPosition = at.x;
				m_selectedKey = key;
				m_trackKey = key;
				break;
			}
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

			KeyMoveEvent keyMoveEvent(sequencer, m_trackKey, offset);
			sequencer->raiseEvent(&keyMoveEvent);
		}
		m_previousPosition = at.x;
	}
}

void Sequence::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
	Rect rcSequence = rc;
	Rect rcTick = rc;

	rcSequence.bottom = rcSequence.top + c_sequenceHeight;
	rcTick.top = rcTick.top + c_sequenceHeight;

	// Save time scale here; it's used in client<->time conversion.
	m_timeScale = sequencer->getTimeScale();

	// Draw sequence background.
	if (!isSelected())
	{
		canvas.setForeground(Color4ub(250, 249, 250));
		canvas.setBackground(Color4ub(238, 237, 240));
		canvas.fillGradientRect(Rect(rc.left, rc.top, separator, rc.bottom));

		canvas.setForeground(Color4ub(170, 169, 170));
		canvas.setBackground(Color4ub(158, 157, 160));
		canvas.fillGradientRect(Rect(separator, rcSequence.top, rcSequence.right, rcSequence.bottom));

		canvas.setForeground(Color4ub(240, 239, 240));
		canvas.setBackground(Color4ub(228, 227, 230));
		canvas.fillGradientRect(Rect(separator, rcTick.top, rcTick.right, rcTick.bottom));
	}
	else
	{
		canvas.setBackground(Color4ub(226, 229, 238));
		canvas.fillRect(Rect(rc.left, rc.top, separator, rc.bottom));

		canvas.setBackground(Color4ub(206, 209, 218));
		canvas.fillRect(Rect(separator, rcSequence.top, rcSequence.right, rcSequence.bottom));

		canvas.setForeground(Color4ub(240, 239, 240));
		canvas.setBackground(Color4ub(228, 227, 230));
		canvas.fillGradientRect(Rect(separator, rcTick.top, rcTick.right, rcTick.bottom));
	}

	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);

	// Draw sequence text.
	canvas.setForeground(Color4ub(0, 0, 0));
	Size ext = canvas.getTextExtent(getName());
	canvas.drawText(
		Point(
			rc.left + 32 + getDepth() * 16,
			rc.top + (rc.getHeight() - ext.cy) / 2
		),
		getName()
	);

	// Draw sequence buttons.
	Rect rcButton;
	rcButton.left = rc.left + separator - c_buttonSize - 4 - int32_t(m_buttons.size()) * (c_buttonSize + 2);
	rcButton.top = rc.top + (rc.getHeight() - c_buttonSize) / 2;
	rcButton.right = rcButton.left + c_buttonSize;
	rcButton.bottom = rcButton.top + c_buttonSize;

	for (int32_t i = 0; i < int32_t(m_buttons.size()); ++i)
	{
		m_buttons[i].rc = rcButton;

		canvas.drawBitmap(
			rcButton.getTopLeft(),
			Point(0, 0),
			Size(16, 16),
			m_buttons[i].state ? m_buttons[i].imageDown.ptr() : m_buttons[i].imageUp.ptr()
		);

		rcButton = rcButton.offset(-c_buttonSize - 2, 0);
	}

	// Draw sequence keys.
	canvas.setClipRect(Rect(
		rc.left + separator,
		rc.top,
		rc.right,
		rc.bottom
	));

	// Draw tickers.
	canvas.setForeground(Color4ub(128, 128, 128));
	int cy = (rcSequence.top + rcSequence.bottom) / 2;
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
