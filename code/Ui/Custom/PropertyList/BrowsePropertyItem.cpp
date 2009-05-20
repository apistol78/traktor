#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"

// Resources
#include "Resources/SmallPen.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.BrowsePropertyItem", BrowsePropertyItem, PropertyItem)

BrowsePropertyItem::BrowsePropertyItem(const std::wstring& text, const Type* filterType, const Guid& value)
:	PropertyItem(text)
,	m_filterType(filterType)
,	m_value(value)
{
}

void BrowsePropertyItem::setFilterType(const Type* filterType)
{
	m_filterType = filterType;
}

const Type* BrowsePropertyItem::getFilterType() const
{
	return m_filterType;
}

void BrowsePropertyItem::setValue(const Guid& value)
{
	m_value = value;
	if (m_buttonEdit)
		m_buttonEdit->setEnable(!m_value.isNull());
}

const Guid& BrowsePropertyItem::getValue() const
{
	return m_value;
}

void BrowsePropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, Bitmap::load(c_ResourceSmallPen, sizeof(c_ResourceSmallPen), L"png"));
	m_buttonEdit->setVisible(visible);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &BrowsePropertyItem::eventEditClick));
	m_buttonEdit->setEnable(!m_value.isNull());
	
	m_buttonBrowse = gc_new< MiniButton >();
	m_buttonBrowse->create(parent, L"...");
	m_buttonBrowse->setVisible(visible);
	m_buttonBrowse->addClickEventHandler(createMethodHandler(this, &BrowsePropertyItem::eventBrowseClick));
}

void BrowsePropertyItem::destroyInPlaceControls()
{
	if (m_buttonEdit)
	{
		m_buttonEdit->destroy();
		m_buttonEdit = 0;
	}
	if (m_buttonBrowse)
	{
		m_buttonBrowse->destroy();
		m_buttonBrowse = 0;
	}
}

void BrowsePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	int width = rc.getHeight();
	outChildRects.push_back(WidgetRect(
		m_buttonEdit,
		Rect(
			rc.right - width * 2,
			rc.top,
			rc.right - width,
			rc.bottom
		)
	));
	outChildRects.push_back(WidgetRect(
		m_buttonBrowse,
		Rect(
			rc.right - width,
			rc.top,
			rc.right,
			rc.bottom
		)
	));
}

void BrowsePropertyItem::showInPlaceControls(bool show)
{
	m_buttonEdit->setVisible(show);
	m_buttonBrowse->setVisible(show);
}

void BrowsePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	std::wstring text;
	if (!getPropertyList()->resolvePropertyGuid(m_value, text))
	{
		if (m_value.isNull())
			return;

		text = m_value.format();
	}

	Font font = getPropertyList()->getFont();
	font.setBold(true);

	canvas.setFont(font);
	canvas.drawText(rc.inflate(-2, -2), text, AnLeft, AnCenter);

	canvas.setFont(getPropertyList()->getFont());
}

void BrowsePropertyItem::eventEditClick(Event* event)
{
	notifyCommand(Command(1, L"Property.Edit"));
}

void BrowsePropertyItem::eventBrowseClick(Event* event)
{
	notifyCommand(Command(2, L"Property.Browse"));
}

		}
	}
}
