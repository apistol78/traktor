#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallPen.h"
#include "Resources/SmallCross.h"
#include "Resources/SmallDots.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.BrowsePropertyItem", BrowsePropertyItem, PropertyItem)

BrowsePropertyItem::BrowsePropertyItem(const std::wstring& text, const TypeInfo* filterType, const Guid& value)
:	PropertyItem(text)
,	m_filterType(filterType)
,	m_value(value)
{
}

void BrowsePropertyItem::setFilterType(const TypeInfo* filterType)
{
	m_filterType = filterType;
}

const TypeInfo* BrowsePropertyItem::getFilterType() const
{
	return m_filterType;
}

void BrowsePropertyItem::setValue(const Guid& value)
{
	m_value = value;
	if (m_buttonEdit)
		m_buttonEdit->setEnable(!m_value.isNull());
	if (m_buttonBrowse)
		m_buttonBrowse->setImage(
			m_value.isNull() ?
				ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png") :
				ui::Bitmap::load(c_ResourceSmallCross, sizeof(c_ResourceSmallCross), L"png")
		);
}

const Guid& BrowsePropertyItem::getValue() const
{
	return m_value;
}

void BrowsePropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, Bitmap::load(c_ResourceSmallPen, sizeof(c_ResourceSmallPen), L"png"));
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &BrowsePropertyItem::eventEditClick);
	m_buttonEdit->setEnable(!m_value.isNull());
	
	T_ASSERT (!m_buttonBrowse);
	m_buttonBrowse = new MiniButton();
	m_buttonBrowse->create(parent,
		m_value.isNull() ?
			ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png") :
			ui::Bitmap::load(c_ResourceSmallCross, sizeof(c_ResourceSmallCross), L"png")
	);
	m_buttonBrowse->addEventHandler< ButtonClickEvent >(this, &BrowsePropertyItem::eventBrowseClick);
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
	if (m_buttonEdit)
		outChildRects.push_back(WidgetRect(
			m_buttonEdit,
			Rect(
				rc.right - width * 2,
				rc.top,
				rc.right - width,
				rc.bottom
			)
		));
	if (m_buttonBrowse)
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

void BrowsePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	std::wstring text;

	bool resolved = getPropertyList()->resolvePropertyGuid(m_value, text);
	if (!resolved)
	{
		if (m_value.isNull())
			return;

		text = m_value.format();
	}

	Font font = getPropertyList()->getFont();
	font.setBold(true);

	canvas.setFont(font);

	Color4ub currentColor = canvas.getForeground();
	if (!resolved)
		canvas.setForeground(Color4ub(255, 0, 0));

	canvas.drawText(rc.inflate(-2, 0), text, AnLeft, AnCenter);

	if (!resolved)
		canvas.setForeground(currentColor);

	canvas.setFont(getPropertyList()->getFont());
}

bool BrowsePropertyItem::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (clipboard)
		return clipboard->setText(m_value.format());
	else
		return false;
}

bool BrowsePropertyItem::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	Guid value(clipboard->getText());
	if (value.isNotNull())
	{
		m_value = value;
		return true;
	}
	else
		return false;
}

void BrowsePropertyItem::eventEditClick(ButtonClickEvent* event)
{
	notifyCommand(Command(1, L"Property.Edit"));
}

void BrowsePropertyItem::eventBrowseClick(ButtonClickEvent* event)
{
	notifyCommand(Command(2, L"Property.Browse"));
}

		}
	}
}
