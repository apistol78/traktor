/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Edit.h"
#include "Ui/MiniButton.h"
#include "Ui/PropertyList/FilePropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

// Resources
#include "Resources/SmallDots.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FilePropertyItem", FilePropertyItem, PropertyItem)

FilePropertyItem::FilePropertyItem(const std::wstring& text, const Path& path)
:	PropertyItem(text)
,	m_path(path)
{
}

void FilePropertyItem::setPath(const Path& path)
{
	m_path = path;
}

const Path& FilePropertyItem::getPath() const
{
	return m_path;
}

void FilePropertyItem::createInPlaceControls(PropertyList* parent)
{
	T_ASSERT(!m_editor);
	m_editor = new Edit();
	m_editor->create(
		parent,
		L"",
		WsNone
	);
	m_editor->setVisible(false);
	m_editor->addEventHandler< FocusEvent >(this, &FilePropertyItem::eventEditFocus);

	T_ASSERT(!m_buttonEdit);
	m_buttonEdit = new MiniButton();
	m_buttonEdit->create(parent, parent->getBitmap(L"UI.SmallDots", c_ResourceSmallDots, sizeof(c_ResourceSmallDots)));
	m_buttonEdit->addEventHandler< ButtonClickEvent >(this, &FilePropertyItem::eventClick);
}

void FilePropertyItem::destroyInPlaceControls()
{
	if (m_buttonEdit)
	{
		m_buttonEdit->destroy();
		m_buttonEdit = nullptr;
	}
	if (m_editor)
	{
		m_editor->destroy();
		m_editor = nullptr;
	}
}

void FilePropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_editor)
		outChildRects.push_back(WidgetRect(
			m_editor,
			Rect(
				rc.left,
				rc.top,
				rc.right - rc.getHeight(),
				rc.bottom
			)
		));
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

void FilePropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	m_editor->setText(m_path.getOriginal());
	m_editor->setVisible(true);
	m_editor->setFocus();
	m_editor->selectAll();
}

void FilePropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, 0), m_path.getOriginal(), AnLeft, AnCenter);
}

bool FilePropertyItem::copy()
{
	if (!m_editor->isVisible(false))
	{
		Clipboard* clipboard = Application::getInstance()->getClipboard();
		if (clipboard)
			return clipboard->setText(m_path.getOriginal());
		else
			return false;
	}
	else
		return false;
}

bool FilePropertyItem::paste()
{
	if (!m_editor->isVisible(false))
	{
		Clipboard* clipboard = Application::getInstance()->getClipboard();
		if (!clipboard)
			return false;

		m_path = Path(clipboard->getText());
		return true;
	}
	else
		return false;
}

void FilePropertyItem::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus())
	{
		m_path = m_editor->getText();
		m_editor->setVisible(false);
		notifyChange();
	}
}

void FilePropertyItem::eventClick(ButtonClickEvent* event)
{
	notifyCommand(Command(L"Property.Browse"));
}

	}
}
