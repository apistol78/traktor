/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

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

ObjectPropertyItem::ObjectPropertyItem(const std::wstring& text, const TypeInfo* objectType, Object* object)
:	PropertyItem(text)
,	m_objectType(objectType)
,	m_object(object)
{
}

void ObjectPropertyItem::setObjectType(const TypeInfo* objectType)
{
	m_objectType = objectType;
}

const TypeInfo* ObjectPropertyItem::getObjectType() const
{
	return m_objectType;
}

void ObjectPropertyItem::setObject(Object* object)
{
	m_object = object;

	if (m_buttonEdit)
	{
		if (m_object)
			m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallCross", c_ResourceSmallCross, sizeof(c_ResourceSmallCross)));
		else
			m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots)));

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
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, L"");
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &ObjectPropertyItem::eventClick);

	if (m_object)
		m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallCross", c_ResourceSmallCross, sizeof(c_ResourceSmallCross)));
	else
		m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots)));
}

void ObjectPropertyItem::destroyInPlaceControls()
{
	safeDestroy(m_buttonEdit);
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
	canvas.drawText(rc.inflate(-2, 0), ss.str(), AnLeft, AnCenter);
}

bool ObjectPropertyItem::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (clipboard)
	{
		ISerializable* serializableObject = dynamic_type_cast< ISerializable* >(m_object);
		if (serializableObject)
			return clipboard->setObject(serializableObject);
		else
			return false;
	}
	else
		return false;
}

bool ObjectPropertyItem::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	Ref< ISerializable > object = clipboard->getObject();
	if (object && (!m_objectType || is_type_of(*m_objectType, type_of(object))))
	{
		m_object = object;
		return true;
	}

	return false;
}

void ObjectPropertyItem::eventClick(ButtonClickEvent* event)
{
	notifyCommand(Command(L"Property.Browse"));
}

		}
	}
}
