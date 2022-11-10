/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/MiniButton.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/PropertyList/PropertyContentChangeEvent.h"
#include "Ui/PropertyList/PropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

// Resources
#include "Resources/PropertyList.h"
#include "Resources/SmallCross.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

Ref< IBitmap > s_imageExpand;
Ref< IBitmap > s_imageCross;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertyItem", PropertyItem, Object)

PropertyItem::PropertyItem(const std::wstring& text)
:	m_propertyList(0)
,	m_text(text)
,	m_visible(true)
,	m_expanded(false)
,	m_selected(false)
,	m_parent(0)
{
	if (!s_imageExpand)
		s_imageExpand = new StyleBitmap(L"UI.PropertyList", c_ResourcePropertyList, sizeof(c_ResourcePropertyList));
	if (!s_imageCross)
		s_imageCross = new StyleBitmap(L"UI.SmallCross", c_ResourceSmallCross, sizeof(c_ResourceSmallCross));
}

void PropertyItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& PropertyItem::getText() const
{
	return m_text;
}

void PropertyItem::setVisible(bool visible)
{
	m_visible = visible;
}

bool PropertyItem::isVisible() const
{
	return m_visible;
}

void PropertyItem::expand()
{
	if (!m_expanded)
	{
		m_expanded = true;
		updateChildrenInPlaceControls();
	}
}

void PropertyItem::collapse()
{
	if (m_expanded)
	{
		m_expanded = false;
		updateChildrenInPlaceControls();
	}
}

bool PropertyItem::isExpanded() const
{
	return m_expanded;
}

bool PropertyItem::isCollapsed() const
{
	return !m_expanded;
}

void PropertyItem::setSelected(bool selected)
{
	m_selected = selected;
}

bool PropertyItem::isSelected() const
{
	return m_selected;
}

int PropertyItem::getDepth() const
{
	int depth = 0;
	for (PropertyItem* parent = m_parent; parent; parent = parent->m_parent)
		++depth;
	return depth;
}

PropertyItem* PropertyItem::getParentItem() const
{
	return m_parent;
}

RefArray< PropertyItem >& PropertyItem::getChildItems()
{
	return m_childItems;
}

const RefArray< PropertyItem >& PropertyItem::getChildItems() const
{
	return m_childItems;
}

void PropertyItem::setPropertyList(PropertyList* propertyList)
{
	m_propertyList = propertyList;
}

PropertyList* PropertyItem::getPropertyList() const
{
	return m_propertyList;
}

void PropertyItem::notifyUpdate()
{
	if (m_propertyList)
		m_propertyList->update();
}

void PropertyItem::notifyCommand(const Command& command)
{
	if (m_propertyList)
	{
		PropertyCommandEvent cmdEvent(m_propertyList, this, command);
		m_propertyList->raiseEvent(&cmdEvent);
	}
}

void PropertyItem::notifyChange()
{
	if (m_propertyList)
	{
		PropertyContentChangeEvent contentChangeEvent(m_propertyList, this);
		m_propertyList->raiseEvent(&contentChangeEvent);
	}
}

void PropertyItem::addChildItem(PropertyItem* childItem)
{
	if (childItem->m_parent == nullptr)
	{
		m_childItems.push_back(childItem);
		childItem->m_parent = this;
	}
}

void PropertyItem::removeChildItem(PropertyItem* childItem)
{
	if (childItem->getParentItem() == this)
	{
		m_childItems.remove(childItem);
		childItem->m_parent = nullptr;
	}
}

bool PropertyItem::needRemoveChildButton() const
{
	return false;
}

void PropertyItem::createInPlaceControls(PropertyList* parent)
{
	if (m_parent && m_parent->needRemoveChildButton())
	{
		m_buttonRemove = new MiniButton();
		m_buttonRemove->create(parent, L"");
		m_buttonRemove->addEventHandler< ButtonClickEvent >(this, &PropertyItem::eventClick);
		m_buttonRemove->setImage(s_imageCross);
	}
}

void PropertyItem::destroyInPlaceControls()
{
	safeDestroy(m_buttonRemove);
}

void PropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_buttonRemove)
		outChildRects.push_back(WidgetRect(
			m_buttonRemove,
			Rect(
				rc.right - rc.getHeight(),
				rc.top,
				rc.right,
				rc.bottom
			)
		));
}

void PropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
}

void PropertyItem::mouseButtonUp(MouseButtonUpEvent* event)
{
}

void PropertyItem::doubleClick(MouseDoubleClickEvent* event)
{
}

void PropertyItem::mouseMove(MouseMoveEvent* event)
{
}

void PropertyItem::paintBackground(Canvas& canvas, const Rect& rc)
{
	const StyleSheet* ss = m_propertyList->getStyleSheet();
	if (m_selected)
	{
		canvas.setBackground(ss->getColor(this, L"background-color-selected"));
		canvas.fillRect(rc);
	}
	else
	{
		canvas.setBackground(ss->getColor(this, L"background-color"));
		canvas.fillRect(rc);
	}
}

void PropertyItem::paintText(Canvas& canvas, const Rect& rc)
{
	int32_t depth = getDepth();
	int32_t left = depth * dpi96(8);

	if (!m_childItems.empty())
	{
		int32_t size = s_imageExpand->getSize().cy;
		int32_t c = (rc.getHeight() - size) / 2;

		canvas.drawBitmap(
			ui::Point(rc.left + left + dpi96(2), rc.top + c),
			ui::Point(m_expanded ? size : 0, 0),
			ui::Size(size, size),
			s_imageExpand,
			BlendMode::Alpha
		);

		left += size + dpi96(4);
	}

	canvas.drawText(
		rc.inflate(-2 - left / 2, -1).offset(left / 2, 0),
		m_text,
		AnLeft,
		AnCenter
	);
}

void PropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
}

bool PropertyItem::copy()
{
	return false;
}

bool PropertyItem::paste()
{
	return false;
}

void PropertyItem::updateChildrenInPlaceControls()
{
	bool expanded = m_expanded;

	for (PropertyItem* parent = m_parent; parent; parent = parent->m_parent)
		expanded &= parent->m_expanded;

	for (auto childItem : m_childItems)
	{
		if (expanded)
			childItem->createInPlaceControls(m_propertyList);
		else
			childItem->destroyInPlaceControls();

		childItem->updateChildrenInPlaceControls();
	}
}

void PropertyItem::eventClick(ButtonClickEvent* event)
{
	notifyCommand(Command(L"Property.Remove"));
}

	}
}
