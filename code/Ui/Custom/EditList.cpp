#include "Core/Log/Log.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Custom/EditListEditEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EditList", EditList, ListBox)

EditList::EditList()
:	m_editId(-1)
,	m_autoAdd(false)
,	m_autoRemove(false)
{
}

bool EditList::create(Widget* parent, int style)
{
	if (!ListBox::create(parent, L"", style))
		return false;

	addEventHandler< MouseDoubleClickEvent >(this, &EditList::eventDoubleClick);

	m_editItem = new Edit();
	m_editItem->create(this, L"", WsBorder);
	m_editItem->hide();
	m_editItem->addEventHandler< FocusEvent >(this, &EditList::eventEditFocus);

	m_editId = -1;

	return true;
}

void EditList::eventDoubleClick(MouseDoubleClickEvent* event)
{
	Point pt = event->getPosition();

	if (m_editId != -1 || m_editItem->isVisible(false))
		return;

	for (int i = 0; i < count(); ++i)
	{
		Rect rc = getItemRect(i);
		if (rc.inside(pt))
		{
			m_editId = i;
			m_editItem->setRect(rc.inflate(0, 2));
			m_editItem->setText(getItem(i));
			m_editItem->selectAll();
			m_editItem->setFocus();
			m_editItem->show();
			break;
		}
	}

	if (m_autoAdd && !m_editItem->isVisible(false))
	{
		// Calculate "next" item rectangle.
		Rect rc = getInnerRect();
		rc.bottom = rc.top + getItemHeight();
		rc = rc.offset(0, count() * getItemHeight());

		m_editItem->setRect(rc.inflate(0, 2));
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
		setFocus();
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
			T_ASSERT (m_autoAdd);
			if (m_editItem->getText().length() > 0)
			{
				EditListEditEvent editEvent(this, m_editItem, -1, m_editItem->getText());
				raiseEvent(&editEvent);
				if (editEvent.consumed())
					add(m_editItem->getText());
			}
		}

		m_editId = -1;
	}
	event->consume();
}

		}
	}
}
