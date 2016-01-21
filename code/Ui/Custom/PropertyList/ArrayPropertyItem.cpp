#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Command.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/NullPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallDots.h"
#include "Resources/SmallPlus.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

Ref< IBitmap > s_imageSmallDots;
Ref< IBitmap > s_imageSmallPlus;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.", ArrayPropertyItem, PropertyItem)

ArrayPropertyItem::ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType, bool readOnly)
:	PropertyItem(text)
,	m_elementType(elementType)
,	m_readOnly(readOnly)
{
	if (!s_imageSmallDots)
		s_imageSmallDots = new ui::StyleBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots));
	if (!s_imageSmallPlus)
		s_imageSmallPlus = new ui::StyleBitmap(L"UI.SmallPlus", c_ResourceSmallPlus, sizeof(c_ResourceSmallPlus));
}

void ArrayPropertyItem::setElementType(const TypeInfo* elementType)
{
	m_elementType = elementType;
}

const TypeInfo* ArrayPropertyItem::getElementType() const
{
	return m_elementType;
}

bool ArrayPropertyItem::needRemoveChildButton() const
{
	return !m_readOnly;
}

void ArrayPropertyItem::createInPlaceControls(Widget* parent)
{
	if (!m_readOnly)
	{
		m_buttonEdit = new MiniButton();
		m_buttonEdit->create(parent, m_elementType ? s_imageSmallDots : s_imageSmallPlus);
		m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &ArrayPropertyItem::eventClick);
	}
}

void ArrayPropertyItem::destroyInPlaceControls()
{
	safeDestroy(m_buttonEdit);
}

void ArrayPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	int32_t dim = rc.getHeight();
	if (m_buttonEdit)
		outChildRects.push_back(WidgetRect(
			m_buttonEdit,
			Rect(
				rc.right - dim,
				rc.top,
				rc.right,
				rc.bottom
			)
		));
}

void ArrayPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	StringOutputStream ss;

	uint32_t elementCount = uint32_t(getChildItems().size());
	ss << L"{ " << elementCount << L" element(s) }";

	canvas.drawText(rc.inflate(-2, 0), ss.str(), AnLeft, AnCenter);
}

void ArrayPropertyItem::eventClick(ButtonClickEvent* event)
{
	if (m_elementType)
		notifyCommand(Command(L"Property.Add"));
	else
	{
		// Add dummy item to indicate where we want to add new element;
		// the ApplyReflector will detect this item and insert the new element.
		addChildItem(new NullPropertyItem());
		notifyCommand(Command(L"Property.Add"));
	}
}

		}
	}
}
