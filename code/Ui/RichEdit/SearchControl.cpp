/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/RichEdit/SearchControl.h"
#include "Ui/RichEdit/SearchEvent.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SearchControl", SearchControl, ui::Container)

bool SearchControl::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 4_ut, 4_ut)))
		return false;

	m_editSearch = new ui::Edit();
	m_editSearch->create(this, L"", ui::WsWantAllInput);
	m_editSearch->addEventHandler< ui::KeyDownEvent >(this, &SearchControl::eventEditSearchKeyDown);
	m_editSearch->addEventHandler< ui::ContentChangeEvent >(this, &SearchControl::eventEditChange);

	m_toolBarMode = new ui::ToolBar();
	m_toolBarMode->create(this);
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchCaseSensitive"));
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchWholeWord"));
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchWildCard"));
	m_toolBarMode->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SearchControl::eventToolClick);

	m_toolCaseSensitive = new ui::ToolBarButton(L"Toggle case sensitive search", 0, ui::Command(L"Script.Editor.ToggleCaseSensitive"), ui::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolCaseSensitive);

	m_toolWholeWord = new ui::ToolBarButton(L"Toggle whole word search", 1, ui::Command(L"Script.Editor.ToggleWholeWord"), ui::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolWholeWord);

	m_toolWildCard = new ui::ToolBarButton(L"Toggle wild-card search", 2, ui::Command(L"Script.Editor.ToggleWildCard"), ui::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolWildCard);

	parent->addEventHandler< ui::ContentChangeEvent >(this, &SearchControl::eventContentChange);

	return true;
}

void SearchControl::setNeedle(const std::wstring& needle)
{
	m_editSearch->setText(needle);
}

std::wstring SearchControl::getNeedle() const
{
	return m_editSearch->getText();
}

bool SearchControl::caseSensitive() const
{
	return m_toolCaseSensitive->isToggled();
}

bool SearchControl::wholeWord() const
{
	return m_toolWholeWord->isToggled();
}

bool SearchControl::wildcard() const
{
	return m_toolWildCard->isToggled();
}

void SearchControl::setAnyMatchingHint(bool hint)
{
	m_editSearch->setBorderColor(!hint ? Color4ub(255, 0, 0, 255) : Color4ub(0, 0, 0, 0));
}

void SearchControl::setFocus()
{
	m_editSearch->selectAll();
	m_editSearch->setFocus();
}

void SearchControl::show()
{
	//if (!isVisible(false))
	//	m_editSearch->setText(m_editor->getSettings()->getProperty< std::wstring >(L"Editor.LastSearch", L""));

	setAnyMatchingHint(true);
	if (!m_editSearch->getText().empty())
	{
		SearchEvent searchEvent(this, true);
		raiseEvent(&searchEvent);
	}

	ui::Container::show();
}

ui::Size SearchControl::getPreferredSize(const Size& hint) const
{
	ui::Size preferedSize = ui::Container::getPreferredSize(hint);
	return ui::Size(pixel(250_ut), preferedSize.cy);
}

void SearchControl::eventEditSearchKeyDown(ui::KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkEscape)
	{
		hide();
		getParent()->setFocus();
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkReturn)
	{
		if (!m_editSearch->getText().empty())
		{
			// Save last search query in user settings.
			//m_editor->checkoutGlobalSettings()->setProperty< PropertyString >(L"Editor.LastSearch", m_editSearch->getText());
			//m_editor->commitGlobalSettings();

			SearchEvent searchEvent(this, false);
			raiseEvent(&searchEvent);
		}
	}
}

void SearchControl::eventEditChange(ui::ContentChangeEvent* event)
{
	setAnyMatchingHint(true);
	if (!m_editSearch->getText().empty())
	{
		SearchEvent searchEvent(this, true);
		raiseEvent(&searchEvent);
	}
}

void SearchControl::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	setAnyMatchingHint(true);
	if (!m_editSearch->getText().empty())
	{
		SearchEvent searchEvent(this, true);
		raiseEvent(&searchEvent);
	}
}

void SearchControl::eventContentChange(ui::ContentChangeEvent* event)
{
	if (!isVisible(false))
		return;

	setAnyMatchingHint(true);
	if (!m_editSearch->getText().empty())
	{
		SearchEvent searchEvent(this, true);
		raiseEvent(&searchEvent);
	}
}

}
