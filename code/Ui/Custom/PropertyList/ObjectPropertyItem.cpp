#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/MethodHandler.h"
#include "Core/Io/StringOutputStream.h"

// Resources
#include "Resources/SmallDots.h"
#include "Resources/SmallCross.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ObjectPropertyItem", ObjectPropertyItem, PropertyItem)

ObjectPropertyItem::ObjectPropertyItem(const std::wstring& text, const Type* objectType, Object* object)
:	PropertyItem(text)
,	m_objectType(objectType)
,	m_object(object)
{
}

void ObjectPropertyItem::setObjectType(const Type* objectType)
{
	m_objectType = objectType;
}

const Type* ObjectPropertyItem::getObjectType() const
{
	return m_objectType;
}

void ObjectPropertyItem::setObject(Object* object)
{
	m_object = object;

	if (m_buttonEdit)
	{
		if (m_object)
			m_buttonEdit->setImage(ui::Bitmap::load(c_ResourceSmallCross, sizeof(c_ResourceSmallCross), L"png"));
		else
			m_buttonEdit->setImage(ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png"));

		m_buttonEdit->update();
	}
}

Ref< Object > ObjectPropertyItem::getObject() const
{
	return m_object;
}

void ObjectPropertyItem::createInPlaceControls(Widget* parent)
{
	T_ASSERT (!m_buttonEdit);
	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, L"");
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &ObjectPropertyItem::eventClick));

	if (m_object)
		m_buttonEdit->setImage(ui::Bitmap::load(c_ResourceSmallCross, sizeof(c_ResourceSmallCross), L"png"));
	else
		m_buttonEdit->setImage(ui::Bitmap::load(c_ResourceSmallDots, sizeof(c_ResourceSmallDots), L"png"));
}

void ObjectPropertyItem::destroyInPlaceControls()
{
	if (m_buttonEdit)
	{
		m_buttonEdit->destroy();
		m_buttonEdit = 0;
	}
}

void ObjectPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_buttonEdit)
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

void ObjectPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	StringOutputStream ss;
	ss << L"{ " << type_name(m_object) << L" }";
	canvas.drawText(rc.inflate(-2, -2), ss.str(), AnLeft, AnCenter);
}

void ObjectPropertyItem::eventClick(Event* event)
{
	notifyCommand(Command(L"Property.Edit"));
}

		}
	}
}
