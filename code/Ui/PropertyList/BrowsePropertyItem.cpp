/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PropertyList/BrowsePropertyItem.h"

#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/MiniButton.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.BrowsePropertyItem", BrowsePropertyItem, PropertyItem)

BrowsePropertyItem::BrowsePropertyItem(const std::wstring& text, const TypeInfo* filterType, const Guid& value)
	: PropertyItem(text)
	, m_filterType(filterType)
	, m_value(value)
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
	if (m_buttonClear)
		m_buttonClear->setEnable(!m_value.isNull());
}

const Guid& BrowsePropertyItem::getValue() const
{
	return m_value;
}

void BrowsePropertyItem::createInPlaceControls(PropertyList* parent)
{
	PropertyItem::createInPlaceControls(parent);

	T_ASSERT(!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, parent->getBitmap(L"UI.SmallPen"));
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &BrowsePropertyItem::eventEditClick);
	m_buttonEdit->setEnable(!m_value.isNull());

	T_ASSERT(!m_buttonClear);
	m_buttonClear = new MiniButton();
	m_buttonClear->create(parent, parent->getBitmap(L"UI.SmallCross"));
	m_buttonClear->addEventHandler< ButtonClickEvent >(this, &BrowsePropertyItem::eventClearClick);
	m_buttonClear->setEnable(!m_value.isNull());

	T_ASSERT(!m_buttonBrowse);
	m_buttonBrowse = new MiniButton();
	m_buttonBrowse->create(parent, parent->getBitmap(L"UI.SmallDots"));
	m_buttonBrowse->addEventHandler< ButtonClickEvent >(this, &BrowsePropertyItem::eventBrowseClick);
}

void BrowsePropertyItem::destroyInPlaceControls()
{
	safeDestroy(m_buttonEdit);
	safeDestroy(m_buttonClear);
	safeDestroy(m_buttonBrowse);
	PropertyItem::destroyInPlaceControls();
}

void BrowsePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	std::vector< WidgetRect > childRects;
	PropertyItem::resizeInPlaceControls(rc, childRects);

	const int32_t width = rc.getHeight();
	const int32_t right = rc.right - (int32_t)childRects.size() * width;

	outChildRects.insert(outChildRects.end(), childRects.begin(), childRects.end());

	int32_t x = right - width;
	if (m_buttonBrowse)
	{
		outChildRects.push_back(WidgetRect(
			m_buttonBrowse,
			Rect(
				x,
				rc.top,
				x + width,
				rc.bottom)));
		x -= width + 2;
	}
	if (m_buttonClear)
	{
		outChildRects.push_back(WidgetRect(
			m_buttonClear,
			Rect(
				x,
				rc.top,
				x + width,
				rc.bottom)));
		x -= width + 2;
	}

	if (m_buttonEdit)
	{
		outChildRects.push_back(WidgetRect(
			m_buttonEdit,
			Rect(
				x,
				rc.top,
				x + width,
				rc.bottom)));
		x -= width + 2;
	}
}

void BrowsePropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	std::wstring text;

	const bool resolved = getPropertyList()->resolvePropertyGuid(m_value, text);
	if (!resolved)
	{
		if (m_value.isNull())
			return;

		text = m_value.format();
	}

	Font font = getPropertyList()->getFont();
	font.setBold(true);

	canvas.setFont(font);

	const Color4ub currentColor = canvas.getForeground();
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

	const Guid value(clipboard->getText());
	if (value.isNotNull())
	{
		setValue(value);
		return true;
	}
	else
		return false;
}

void BrowsePropertyItem::eventEditClick(ButtonClickEvent* event)
{
	notifyCommand(Command(1, L"Property.Edit"));
}

void BrowsePropertyItem::eventClearClick(ButtonClickEvent* event)
{
	notifyCommand(Command(2, L"Property.Clear"));
}

void BrowsePropertyItem::eventBrowseClick(ButtonClickEvent* event)
{
	notifyCommand(Command(3, L"Property.Browse"));
}

}
