/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/MiniButton.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/NullPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallDots.h"
#include "Resources/SmallPlus.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.", ArrayPropertyItem, PropertyItem)

ArrayPropertyItem::ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType, bool readOnly)
:	PropertyItem(text)
,	m_elementType(elementType)
,	m_readOnly(readOnly)
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

bool ArrayPropertyItem::needRemoveChildButton() const
{
	return !m_readOnly;
}

void ArrayPropertyItem::createInPlaceControls(PropertyList* parent)
{
	if (!m_readOnly)
	{
		Ref< ui::IBitmap > img;
		if (m_elementType)
			img = parent->getBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots));
		else
			img = parent->getBitmap(L"UI.SmallPlus", c_ResourceSmallPlus, sizeof(c_ResourceSmallPlus));

		m_buttonEdit = new MiniButton();
		m_buttonEdit->create(parent, img);
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

bool ArrayPropertyItem::paste()
{
	if (!m_elementType)
		return false;

	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	Ref< ISerializable > object = clipboard->getObject();
	if (object && is_type_of(*m_elementType, type_of(object)))
	{
		notifyCommand(Command(L"Property.Add", object));
		return true;
	}

	return false;
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
