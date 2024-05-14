/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/EditList.h"
#include "Ui/EditListEditEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EditList", EditList, ListBox)

EditList::EditList()
:	m_editId(-1)
,	m_autoAdd(false)
,	m_autoRemove(false)
{
}

bool EditList::create(Widget* parent, uint32_t style)
{
	if (!ListBox::create(parent, style))
		return false;

	addEventHandler< MouseDoubleClickEvent >(this, &EditList::eventDoubleClick);

	m_editItem = new Edit();
	m_editItem->create(this, L"", WsBorder);
	m_editItem->hide();
	m_editItem->addEventHandler< FocusEvent >(this, &EditList::eventEditFocus);

	m_editId = -1;
	m_autoAdd = bool((style & WsAutoAdd) != 0);
	m_autoRemove = bool((style & WsAutoRemove) != 0);

	return true;
}

void EditList::eventDoubleClick(MouseDoubleClickEvent* event)
{
	Point pt = getClientPosition(event->getPosition());

	if (m_editId != -1 || m_editItem->isVisible(false))
		return;

	bool foundItem = false;

	for (int i = 0; i < count(); ++i)
	{
		const Rect rc = getItemRect(i);
		if (rc.inside(pt))
		{
			m_editId = i;
			m_editItem->setRect(rc.offset(getScrollOffset()).inflate(0, 2));
			m_editItem->setText(getItem(i));
			m_editItem->selectAll();
			m_editItem->setFocus();
			m_editItem->show();
			foundItem = true;
			break;
		}
	}

	if (m_autoAdd && !foundItem)
	{
		// Calculate "next" item rectangle.
		Rect rc = getInnerRect();
		rc.bottom = rc.top + getItemHeight();
		rc = rc.offset(0, count() * getItemHeight());

		m_editItem->setRect(rc.offset(getScrollOffset()).inflate(0, 2));
		m_editItem->setText(L"");
		m_editItem->setFocus();
		m_editItem->show();

		m_editId = -1;
	}

	event->consume();
}

void EditList::eventEditFocus(FocusEvent* event)
{
	if (m_editItem->isVisible(false) && event->lostFocus())
	{
#if !defined(__LINUX__) && !defined(__RPI__)
		setFocus();
#endif
		m_editItem->hide();

		if (m_editId >= 0)
		{
			if (m_editItem->getText().length() > 0)
			{
				EditListEditEvent editEvent(this, m_editItem, m_editId, m_editItem->getText());
				raiseEvent(&editEvent);
				if (editEvent.consumed())
					setItem(m_editId, m_editItem->getText());
			}
			else	// Cleared, remove item
			{
				if (m_autoRemove)
				{
					EditListEditEvent editEvent(this, m_editItem, m_editId, L"");
					raiseEvent(&editEvent);
					if (editEvent.consumed())
						remove(m_editId);
				}
			}
		}
		else
		{
			T_ASSERT(m_autoAdd);
			if (m_editItem->getText().length() > 0)
			{
				EditListEditEvent editEvent(this, m_editItem, -1, m_editItem->getText());
				raiseEvent(&editEvent);
				if (editEvent.consumed())
					add(m_editItem->getText());
			}
		}

		m_editId = -1;
		event->consume();
	}
}

}
