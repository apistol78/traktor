/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Sequencer/Sequence.h"
#include "Ui/Sequencer/SequenceButtonClickEvent.h"
#include "Ui/Sequencer/SequencerControl.h"
#include "Ui/Sequencer/Key.h"
#include "Ui/Sequencer/KeyMoveEvent.h"
#include "Ui/Sequencer/KeySelectEvent.h"

namespace traktor::ui
{
	namespace
	{

const int c_buttonSize = 18;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Sequence", Sequence, SequenceItem)

Sequence::Sequence(const std::wstring& name)
:	SequenceItem(name)
,	m_previousPosition(0)
,	m_timeScale(8)
{
}

int32_t Sequence::addButton(IBitmap* imageUp, IBitmap* imageDown, const Command& command, bool toggle)
{
	Button btn;
	btn.imageUp = imageUp;
	btn.imageDown = imageDown;
	btn.command = command;
	btn.toggle = toggle;
	btn.state = false;
	m_buttons.push_back(btn);
	return int32_t(m_buttons.size() - 1);
}

void Sequence::setButtonState(int32_t buttonIndex, bool state)
{
	m_buttons[buttonIndex].state = state;
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

void Sequence::setSelectedKey(Key* key)
{
	m_selectedKey = (key != nullptr && containsKey(key)) ? key : nullptr;
}

int Sequence::clientFromTime(int time) const
{
	return time / m_timeScale;
}

int Sequence::timeFromClient(int client) const
{
	return client * m_timeScale;
}

bool Sequence::setSelected(bool selected)
{
	// A key is part of the selection of its sequence.
	if (!selected)
		m_selectedKey = nullptr;

	return SequenceItem::setSelected(selected);
}

bool Sequence::isTrackingKey() const
{
	return m_trackKey != nullptr;
}

void Sequence::mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	if (at.x < separator)
	{
		for (int32_t i = 0; i < int32_t(m_buttons.size()); ++i)
		{
			if (m_buttons[i].rc.inside(Point(at.x + rc.left, at.y + rc.top)))
			{
				if (m_buttons[i].toggle)
				{
					// Toggle button state.
					m_buttons[i].state = !m_buttons[i].state;
				}

				// Notify button listeners.
				SequenceButtonClickEvent clickEvent(sequencer, this, m_buttons[i].command);
				sequencer->raiseEvent(&clickEvent);
				break;
			}
		}
	}
	else
	{
		m_selectedKey = nullptr;
		m_trackKey = nullptr;
		m_trackKeyMoved = false;

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
			key->getRect(sequencer, this, rcClient, rcKey);

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
	// A key is selected by clicking it; a press is also the start of a drag.
	const bool clicked = (m_trackKey != nullptr && !m_trackKeyMoved);

	m_previousPosition = 0;
	m_trackKey = nullptr;
	m_trackKeyMoved = false;

	if (clicked)
	{
		KeySelectEvent keySelectEvent(sequencer, this, m_selectedKey);
		sequencer->raiseEvent(&keySelectEvent);
	}
}

void Sequence::mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	if (button && m_trackKey)
	{
		int32_t offset = timeFromClient(at.x - m_previousPosition);
		if (offset != 0)
		{
			m_trackKey->move(offset);
			m_trackKeyMoved = true;

			KeyMoveEvent keyMoveEvent(sequencer, m_trackKey, offset);
			sequencer->raiseEvent(&keyMoveEvent);
		}
		m_previousPosition = at.x;
	}
}

void Sequence::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
	const StyleSheet* ss = sequencer->getStyleSheet();

	const bool enabled = sequencer->isEnable(true);

	// Save time scale here; it's used in client<->time conversion.
	m_timeScale = sequencer->getTimeScale();

	// Draw sequence background.
	const wchar_t* background = L"background-color";
	if (!enabled)
		background = L"background-color-disabled";
	else if (isSelected())
		background = L"background-color-selected";

	canvas.setBackground(ss->getColor(this, background));
	canvas.fillRect(Rect(separator, rc.top, rc.right, rc.bottom));

	canvas.setForeground(ss->getColor(this, L"line-color"));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);

	// Draw sequence text.
	const Size ext = canvas.getFontMetric().getExtent(getName());
	canvas.setForeground(ss->getColor(this, enabled ? L"color" : L"color-disabled"));
	canvas.drawText(
		Point(
			rc.left + sequencer->pixel(Unit(32 + getDepth() * 16)),
			rc.top + (rc.getHeight() - ext.cy) / 2
		),
		getName()
	);

	// Draw sequence buttons.
	const int32_t buttonSize = rc.getHeight();

	Rect rcButton;
	rcButton.left = rc.left + separator - (int32_t)m_buttons.size() * buttonSize;
	rcButton.top = rc.top;
	rcButton.right = rcButton.left + buttonSize;
	rcButton.bottom = rc.bottom;

	for (int32_t i = 0; i < (int32_t)m_buttons.size(); ++i)
	{
		m_buttons[i].rc = rcButton;

		auto image = m_buttons[i].state ? m_buttons[i].imageDown.ptr() : m_buttons[i].imageUp.ptr();
		if (image)
		{
			const Size size = image->getSize(sequencer);
			const Size offset(
				(buttonSize - size.cx) / 2,
				(buttonSize - size.cy) / 2
			);
			canvas.drawBitmap(
				rcButton.getTopLeft() + offset,
				Point(0, 0),
				size,
				image,
				ui::BlendMode::Alpha
			);
		}

		rcButton = rcButton.offset(buttonSize, 0);
	}

	// Draw sequence keys.
	canvas.setClipRect(Rect(
		rc.left + separator,
		rc.top,
		rc.right,
		rc.bottom
	));

	// Draw tickers.
	canvas.setForeground(ss->getColor(this, enabled ? L"tick-color" : L"color-disabled"));
	const int32_t cy = (rc.top + rc.bottom) / 2;
	for (int32_t i = 100; i < sequencer->getLength(); i += 100)
	{
		const int32_t cx = separator + clientFromTime(i) - scrollOffset;
		if (cx > rc.right)
			break;
		const int32_t cya = (i % 1000 == 0) ? 4 : 0;
		canvas.drawLine(cx, cy - sequencer->pixel(Unit(2 + cya)), cx, cy + sequencer->pixel(Unit(2 + cya)));
	}

	for (auto key : m_keys)
	{
		const Rect rcClient(
			rc.left + separator,
			rc.top,
			rc.right,
			rc.bottom
		);
		key->paint(sequencer, canvas, this, rcClient, scrollOffset);
	}
}

}
