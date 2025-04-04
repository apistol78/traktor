/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/StyleBitmap.h"
#include "Ui/MiniButton.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ObjectPropertyItem", ObjectPropertyItem, PropertyItem)

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
			m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallCross"));
		else
			m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallDots"));

		m_buttonEdit->update();
	}
}

Ref< Object > ObjectPropertyItem::getObject() const
{
	return m_object;
}

void ObjectPropertyItem::createInPlaceControls(PropertyList* parent)
{
	T_ASSERT(!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, L"");
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &ObjectPropertyItem::eventClick);

	if (m_object)
		m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallCross"));
	else
		m_buttonEdit->setImage(new ui::StyleBitmap(L"UI.SmallDots"));
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

void ObjectPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
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
