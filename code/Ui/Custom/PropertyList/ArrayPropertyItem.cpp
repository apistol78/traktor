#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/NullPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Core/Io/StringOutputStream.h"

// Resources
#include "Resources/SmallDots.h"
#include "Resources/SmallPlus.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.", ArrayPropertyItem, PropertyItem)

ArrayPropertyItem::ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType)
:	PropertyItem(text)
,	m_elementType(elementType)
{
}

void ArrayPropertyItem::setElementType(const TypeInfo* elementType)
{
	m_elementType = elementType;
}

const TypeInfo* ArrayPropertyItem::getElementType() const
{
	return m_elementType;
}

void ArrayPropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(
		parent,
		m_elementType ? 
			ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png") : 
			ui::Bitmap::load(c_ResourceSmallPlus, sizeof(c_ResourceSmallPlus), L"png")
	);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &ArrayPropertyItem::eventClick));
}

void ArrayPropertyItem::destroyInPlaceControls()
{
	if (m_buttonEdit)
	{
		m_buttonEdit->destroy();
		m_buttonEdit = 0;
	}
}

void ArrayPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_buttonEdit)
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
}

void ArrayPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	StringOutputStream ss;

	uint32_t elementCount = uint32_t(getChildItems().size());
	ss << L"{ " << elementCount << L" element(s) }";

	canvas.drawText(rc.inflate(-2, -2), ss.str(), AnLeft, AnCenter);
}

void ArrayPropertyItem::eventClick(Event* event)
{
	if (m_elementType)
		notifyCommand(Command(L"Property.Edit"));
	else
	{
		// Add dummy item to indicate where we want to add new element;
		// the ApplyReflector will detect this item and insert the new element.
		addChildItem(new NullPropertyItem());
		notifyCommand(Command(L"Property.Edit"));
	}
}

		}
	}
}
