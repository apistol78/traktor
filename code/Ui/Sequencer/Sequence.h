/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Ui/Command.h"
#include "Ui/Rect.h"
#include "Ui/Sequencer/SequenceItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Command;
class IBitmap;
class Key;

/*! Sequence of keys.
 * \ingroup UI
 */
class T_DLLCLASS Sequence : public SequenceItem
{
	T_RTTI_CLASS;

public:
	explicit Sequence(const std::wstring& name);

	int32_t addButton(IBitmap* imageUp, IBitmap* imageDown, const Command& command, bool toggle = false);

	void setButtonState(int32_t buttonIndex, bool state);

	bool getButtonState(int32_t buttonIndex) const;

	void addKey(Key* key);

	void removeKey(Key* key);

	void removeAllKeys();

	bool containsKey(Key* key) const;

	const RefArray< Key >& getKeys() const;

	Ref< Key > getSelectedKey() const;

	int clientFromTime(int time) const;

	int timeFromClient(int client) const;

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override;

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override;

	virtual void mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) override;

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset) override;

private:
	struct Button
	{
		Ref< IBitmap > imageUp;
		Ref< IBitmap > imageDown;
		Command command;
		bool toggle;
		bool state;
		Rect rc;
	};

	RefArray< Key > m_keys;
	AlignedVector< Button > m_buttons;
	Ref< Key > m_selectedKey;
	Ref< Key > m_trackKey;
	int32_t m_previousPosition;
	int32_t m_timeScale;
};

}
