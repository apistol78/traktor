#include <Ui/MethodHandler.h>
#include <Ui/Events/MouseEvent.h>
#include <Ui/Events/FocusEvent.h>
#include <Ui/Events/EditEvent.h>
#include <Core/Log/Log.h>
#include "EditList.h"
using namespace traktor;

// It seems creating in-place edit control crashes in wxGTK.
#if !defined(_WIN32)
#	define T_NO_INPLACE_EDIT
#endif

bool EditList::create(ui::Widget* parent)
{
	if (!ListBox::create(parent))
		return false;

	addDoubleClickEventHandler(
		gc_new< ui::MethodHandler< EditList > >(
			this,
			&EditList::eventDoubleClick
		)
	);

#if !defined(T_NO_INPLACE_EDIT)
	m_editItem = gc_new< ui::Edit >();
	m_editItem->create(this, L"", ui::WsBorder);
	m_editItem->hide();
	m_editItem->addFocusEventHandler(
		gc_new< ui::MethodHandler< EditList > >(
			this,
			&EditList::eventEditFocus
		)
	);
#endif

	m_editId = -1;

	return true;
}

void EditList::addEditEventHandler(traktor::ui::EventHandler* eventHandler)
{
	addEventHandler(ui::EiContentChange, eventHandler);
}

void EditList::eventDoubleClick(ui::Event* event)
{
	ui::Point pt = static_cast< ui::MouseEvent* >(event)->getPosition();

#if !defined(T_NO_INPLACE_EDIT)
	if (m_editId != -1 || m_editItem->isVisible(false))
		return;

	for (int i = 0; i < count(); ++i)
	{
		ui::Rect rc = getItemRect(i);
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

	if (!m_editItem->isVisible(false))
	{
		// Calculate "next" item rectangle.
		ui::Rect rc = getInnerRect();
		rc.bottom = rc.top + getItemHeight();
		rc = rc.offset(0, count() * getItemHeight());
		
		m_editItem->setRect(rc.inflate(0, 2));
		m_editItem->setText(L"");
		m_editItem->setFocus();
		m_editItem->show();

		m_editId = -1;
	}
#endif

	event->consume();
}

void EditList::eventEditFocus(ui::Event* event)
{
#if !defined(T_NO_INPLACE_EDIT)
	if (m_editItem->isVisible(false) && static_cast< ui::FocusEvent* >(event)->lostFocus())
	{
		setFocus();
		m_editItem->hide();
		
		if (m_editId >= 0)
		{
			if (m_editItem->getText().length() > 0)
			{
				setItem(m_editId, m_editItem->getText());

				ui::EditEvent editEvent(this, m_editItem, m_editId, m_editItem->getText());
				raiseEvent(ui::EiContentChange, &editEvent);
			}
			else	// Cleared, remove item
			{
				remove(m_editId);

				ui::EditEvent editEvent(this, m_editItem, m_editId, L"");
				raiseEvent(ui::EiContentChange, &editEvent);
			}
		}
		else
		{
			if (m_editItem->getText().length() > 0)
			{
				add(m_editItem->getText());

				ui::EditEvent editEvent(this, m_editItem, -1, m_editItem->getText());
				raiseEvent(ui::EiContentChange, &editEvent);
			}
		}

		m_editId = -1;
	}
#endif
	event->consume();
}

