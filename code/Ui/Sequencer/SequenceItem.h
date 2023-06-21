/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Ui/Associative.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Canvas;
class SequencerControl;

/*! Sequence item.
 * \ingroup UI
 */
class T_DLLCLASS SequenceItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	SequenceItem(const std::wstring& name);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	bool setSelected(bool selected);

	bool isSelected() const;

	SequenceItem* getParentItem() const;

	void addChildItem(SequenceItem* childItem);

	void addChildItemBefore(SequenceItem* beforeChildItem, SequenceItem* childItem);

	void removeChildItem(SequenceItem* childItem);

	RefArray< SequenceItem >& getChildItems();

	int32_t getDepth() const;

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) = 0;

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) = 0;

	virtual void mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset) = 0;

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset) = 0;

private:
	std::wstring m_name;
	bool m_selected;
	Ref< SequenceItem > m_parent;
	RefArray< SequenceItem > m_childItems;
};

	}
}

