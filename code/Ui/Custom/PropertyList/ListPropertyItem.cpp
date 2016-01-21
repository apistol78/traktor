#include "Ui/Canvas.h"
#include "Ui/FloodLayout.h"
#include "Ui/ListBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/ToolForm.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PropertyList/ListPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallDots.h"

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

ListPropertyItem::~ListPropertyItem()
{
	T_ASSERT (!m_buttonDrop);
	T_ASSERT (!m_listForm);
	T_ASSERT (!m_listBox);
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
	m_buttonDrop = new MiniButton();
	m_buttonDrop->create(parent, new ui::StyleBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots)));
	m_buttonDrop->addEventHandler< ButtonClickEvent >(this, &ListPropertyItem::eventDropClick);

	T_ASSERT (!m_listForm);
	m_listForm = new ToolForm();
	m_listForm->create(parent, L"List", 0, 0, WsNone, new ui::FloodLayout());
	m_listForm->setVisible(false);

	T_ASSERT (!m_listBox);
	m_listBox = new ListBox();
	m_listBox->create(m_listForm);
	m_listBox->addEventHandler< SelectionChangeEvent >(this, &ListPropertyItem::eventSelect);
	m_listBox->addEventHandler< FocusEvent >(this, &ListPropertyItem::eventFocus);

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
	canvas.drawText(rc.inflate(-2, 0), value, AnLeft, AnCenter);
}

void ListPropertyItem::eventDropClick(ButtonClickEvent* event)
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

void ListPropertyItem::eventSelect(SelectionChangeEvent* event)
{
	m_listForm->setVisible(false);
	notifyChange();
	notifyUpdate();
}

void ListPropertyItem::eventFocus(FocusEvent* event)
{
	if (event->lostFocus())
	{
		m_selected = m_listBox->getSelected();
		m_listForm->setVisible(false);
	}
}

		}
	}
}
