#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/MethodHandler.h"
#include "Core/Io/StringOutputStream.h"

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
}

Object* ObjectPropertyItem::getObject() const
{
	return m_object;
}

void ObjectPropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	m_buttonEdit = gc_new< MiniButton >();
	m_buttonEdit->create(parent, L"...");
	m_buttonEdit->setVisible(visible);
	m_buttonEdit->addClickEventHandler(createMethodHandler(this, &ObjectPropertyItem::eventClick));
}

void ObjectPropertyItem::destroyInPlaceControls()
{
	m_buttonEdit->destroy();
}

void ObjectPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
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

void ObjectPropertyItem::showInPlaceControls(bool show)
{
	m_buttonEdit->setVisible(show);
}

void ObjectPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	StringOutputStream ss;
	ss << L"{ " << type_name(m_object) << L" }";
	canvas.drawText(rc.inflate(-2, -2), ss.str(), AnLeft, AnCenter);
}

void ObjectPropertyItem::eventClick(Event* event)
{
	notifyCommand();
}

		}
	}
}
