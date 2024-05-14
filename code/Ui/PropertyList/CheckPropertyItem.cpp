/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/StyleBitmap.h"
#include "Ui/PropertyList/CheckPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CheckProperyItem", CheckPropertyItem, PropertyItem)

CheckPropertyItem::CheckPropertyItem(const std::wstring& text, bool checked)
:	PropertyItem(text)
,	m_checked(checked)
{
	m_imageUnchecked = new StyleBitmap(L"UI.Unchecked");
	m_imageChecked = new StyleBitmap(L"UI.Checked");
}

void CheckPropertyItem::setChecked(bool checked)
{
	m_checked = checked;
}

bool CheckPropertyItem::isChecked() const
{
	return m_checked;
}

void CheckPropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	m_checked = !m_checked;
	notifyChange();
}

void CheckPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	IBitmap* image = m_checked ? m_imageChecked : m_imageUnchecked;
	T_ASSERT(image);

	int c = (rc.getHeight() - image->getSize(parent).cy) / 2;
	canvas.drawBitmap(
		ui::Point(rc.left + 2, rc.top + c),
		ui::Point(0, 0),
		image->getSize(parent),
		image,
		BlendMode::Alpha
	);
}

bool CheckPropertyItem::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (clipboard)
		return clipboard->setText(m_checked ? L"true" : L"false");
	else
		return false;	
}

bool CheckPropertyItem::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	m_checked = (bool)(compareIgnoreCase(clipboard->getText(), L"true") == 0);
	return true;
}

}
