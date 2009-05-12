#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/MethodHandler.h"

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
}

const Guid& BrowsePropertyItem::getValue() const
{
	return m_value;
}

void BrowsePropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, L"...");
	m_buttonEdit->setVisible(visible);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &BrowsePropertyItem::eventClick));
}

void BrowsePropertyItem::destroyInPlaceControls()
{
	m_buttonEdit->destroy();
}

void BrowsePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	outChildRects.push_back(WidgetRect(
		m_buttonEdit,
		Rect(
			rc.right - rc.getHeight(),
			rc.top,
			rc.right,
			rc.bottom
		)
	));
}

void BrowsePropertyItem::showInPlaceControls(bool show)
{
	m_buttonEdit->setVisible(show);
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

void BrowsePropertyItem::eventClick(Event* event)
{
	notifyCommand();
}

		}
	}
}
