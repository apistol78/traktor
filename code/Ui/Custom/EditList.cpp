#include "Core/Log/Log.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/EditList.h"
#include "Ui/Events/EditEvent.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/MouseEvent.h"

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

	addDoubleClickEventHandler(
		new MethodHandler< EditList >(
			this,
			&EditList::eventDoubleClick
		)
	);

	m_editItem = new Edit();
	m_editItem->create(this, L"", WsBorder);
	m_editItem->hide();
	m_editItem->addFocusEventHandler(
		new MethodHandler< EditList >(
			this,
			&EditList::eventEditFocus
		)
	);

	m_editId = -1;

	return true;
}

void EditList::addEditEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiContentChange, eventHandler);
}

void EditList::eventDoubleClick(Event* event)
{
	Point pt = static_cast< MouseEvent* >(event)->getPosition();

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

void EditList::eventEditFocus(Event* event)
{
	if (m_editItem->isVisible(false) && static_cast< FocusEvent* >(event)->lostFocus())
	{
		setFocus();
		m_editItem->hide();
		
		if (m_editId >= 0)
		{
			if (m_editItem->getText().length() > 0)
			{
				EditEvent editEvent(this, m_editItem, m_editId, m_editItem->getText());
				raiseEvent(EiContentChange, &editEvent);
				if (editEvent.consumed())
					setItem(m_editId, m_editItem->getText());
			}
			else	// Cleared, remove item
			{
				if (m_autoRemove)
				{
					EditEvent editEvent(this, m_editItem, m_editId, L"");
					raiseEvent(EiContentChange, &editEvent);
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
				EditEvent editEvent(this, m_editItem, -1, m_editItem->getText());
				raiseEvent(EiContentChange, &editEvent);
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
