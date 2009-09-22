#include "Ui/Custom/PropertyList/ListPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/ToolForm.h"
#include "Ui/ListBox.h"
#include "Ui/Canvas.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/FocusEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ListPropertyItem", ListPropertyItem, PropertyItem)

ListPropertyItem::ListPropertyItem(const std::wstring& text)
:	PropertyItem(text)
,	m_selected(-1)
{
}

int ListPropertyItem::add(const std::wstring& item)
{
	if (m_listBox)
		m_listBox->add(item);

	m_items.push_back(item);
	return int(m_items.size() - 1);
}

bool ListPropertyItem::remove(int index)
{
	if (index >= int32_t(m_items.size()))
		return false;

	if (m_listBox)
		m_listBox->remove(index);

	std::vector< std::wstring >::iterator i = m_items.begin() + index;
	m_items.erase(i);

	if (index >= m_selected)
		m_selected = -1;

	return true;
}

void ListPropertyItem::removeAll()
{
	if (m_listBox)
		m_listBox->removeAll();

	m_items.resize(0);
	m_selected = -1;
}

int ListPropertyItem::count() const
{
	return int32_t(m_items.size());
}

std::wstring ListPropertyItem::get(int index) const
{
	return (index >= 0 && index < int32_t(m_items.size())) ? m_items[index] : L"";
}

void ListPropertyItem::select(int index)
{
	m_selected = index;
}

int ListPropertyItem::getSelected() const
{
	return m_selected;
}

std::wstring ListPropertyItem::getSelectedItem() const
{
	return get(m_selected);
}

void ListPropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_buttonDrop);
	m_buttonDrop = gc_new< MiniButton >();
	m_buttonDrop->create(parent, L"...");
	m_buttonDrop->addClickEventHandler(createMethodHandler(this, &ListPropertyItem::eventDropClick));

	T_ASSERT (!m_listForm);
	m_listForm = gc_new< ToolForm >();
	m_listForm->create(parent, L"List", 0, 0, WsNone, gc_new< ui::FloodLayout >());
	m_listForm->setVisible(false);

	T_ASSERT (!m_listBox);
	m_listBox = gc_new< ListBox >();
	m_listBox->create(m_listForm);
	m_listBox->addSelectEventHandler(createMethodHandler(this, &ListPropertyItem::eventSelect));
	m_listBox->addFocusEventHandler(createMethodHandler(this, &ListPropertyItem::eventFocus));

	for (std::vector< std::wstring >::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
		m_listBox->add(*i);

	m_listBox->select(m_selected);
}

void ListPropertyItem::destroyInPlaceControls()
{
	if (m_buttonDrop)
	{
		m_buttonDrop->destroy();
		m_buttonDrop = 0;
	}

	if (m_listBox)
	{
		m_selected = m_listBox->getSelected();
		m_listBox->destroy();
		m_listBox = 0;
	}

	if (m_listForm)
	{
		m_listForm->destroy();
		m_listForm = 0;
	}
}

void ListPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_buttonDrop)
		outChildRects.push_back(WidgetRect(
			m_buttonDrop,
			Rect(
				rc.right - rc.getHeight(),
				rc.top,
				rc.right,
				rc.bottom
			)
		));

	m_listRect = Rect(
		rc.left,
		rc.top + rc.getHeight(),
		rc.right,
		rc.top + rc.getHeight() + 16 * 4
	);
}

void ListPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	std::wstring value = getSelectedItem();
	canvas.drawText(rc.inflate(-2, -2), value, AnLeft, AnCenter);
}

void ListPropertyItem::eventDropClick(Event* event)
{
	if (!m_listForm->isVisible(false))
	{
		Point topLeft = m_listForm->getParent()->clientToScreen(m_listRect.getTopLeft());
		Rect listRect = m_listRect; listRect.moveTo(topLeft);

		m_listForm->setRect(listRect);
		m_listForm->setVisible(true);
		m_listForm->raise();
		m_listBox->setFocus();
	}
	else
	{
		m_listForm->setVisible(false);
	}
}

void ListPropertyItem::eventSelect(Event* event)
{
	m_listForm->setVisible(false);
	notifyChange();
	notifyUpdate();
}

void ListPropertyItem::eventFocus(Event* event)
{
	if (static_cast< FocusEvent* >(event)->lostFocus())
	{
		m_selected = m_listBox->getSelected();
		m_listForm->setVisible(false);
	}
}

		}
	}
}
