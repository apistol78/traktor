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

ListPropertyItem::ListPropertyItem(const std::wstring& text) :
	PropertyItem(text)
{
}

int ListPropertyItem::add(const std::wstring& item)
{
	return m_listBox->add(item);
}

bool ListPropertyItem::remove(int index)
{
	return m_listBox->remove(index);
}

void ListPropertyItem::removeAll()
{
	m_listBox->removeAll();
}

int ListPropertyItem::count() const
{
	return m_listBox->count();
}

std::wstring ListPropertyItem::get(int index) const
{
	return m_listBox->getItem(index);
}

void ListPropertyItem::select(int index)
{
	m_listBox->select(index);
}

int ListPropertyItem::getSelected() const
{
	return m_listBox->getSelected();
}

std::wstring ListPropertyItem::getSelectedItem() const
{
	return m_listBox->getSelectedItem();
}

void ListPropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	m_buttonDrop = gc_new< MiniButton >();
	m_buttonDrop->create(parent, L"...");
	m_buttonDrop->setVisible(visible);
	m_buttonDrop->addClickEventHandler(createMethodHandler(this, &ListPropertyItem::eventDropClick));

	m_listForm = gc_new< ToolForm >();
	m_listForm->create(parent, L"List", 0, 0, WsNone, gc_new< ui::FloodLayout >());
	m_listForm->setVisible(false);

	m_listBox = gc_new< ListBox >();
	m_listBox->create(m_listForm);
	m_listBox->addSelectEventHandler(createMethodHandler(this, &ListPropertyItem::eventSelect));
	m_listBox->addFocusEventHandler(createMethodHandler(this, &ListPropertyItem::eventFocus));
}

void ListPropertyItem::destroyInPlaceControls()
{
	m_buttonDrop->destroy();
	m_buttonDrop = 0;

	m_listBox->destroy();
	m_listBox = 0;
}

void ListPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
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

void ListPropertyItem::showInPlaceControls(bool show)
{
	m_buttonDrop->setVisible(show);
}

void ListPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	if (m_listBox)
	{
		int selected = m_listBox->getSelected();
		std::wstring value = selected >= 0 ? m_listBox->getItem(selected) : L"";
		canvas.drawText(rc.inflate(-2, -2), value, AnLeft, AnCenter);
	}
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
		m_listForm->setVisible(false);
}

		}
	}
}
