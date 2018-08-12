/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/RichEdit/SearchControl.h"
#include "Ui/Custom/RichEdit/SearchEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SearchControl", SearchControl, ui::Container)

SearchControl::SearchControl()
{
}

bool SearchControl::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", ui::dpi96(4), ui::dpi96(4))))
		return false;

	m_editSearch = new ui::Edit();
	m_editSearch->create(this, L"", ui::WsBorder | ui::WsWantAllInput);
	m_editSearch->addEventHandler< ui::KeyDownEvent >(this, &SearchControl::eventEditSearchKeyDown);
	m_editSearch->addEventHandler< ui::ContentChangeEvent >(this, &SearchControl::eventEditChange);

	m_toolBarMode = new ui::custom::ToolBar();
	m_toolBarMode->create(this);
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchCaseSensitive"), 1);
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchWholeWord"), 1);
	m_toolBarMode->addImage(new ui::StyleBitmap(L"UI.SearchWildCard"), 1);
	m_toolBarMode->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SearchControl::eventToolClick);
	
	m_toolCaseSensitive = new ui::custom::ToolBarButton(L"Toggle case sensitive search", 0, ui::Command(L"Script.Editor.ToggleCaseSensitive"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolCaseSensitive);

	m_toolWholeWord = new ui::custom::ToolBarButton(L"Toggle whole word search", 1, ui::Command(L"Script.Editor.ToggleWholeWord"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolWholeWord);

	m_toolWildCard = new ui::custom::ToolBarButton(L"Toggle wild-card search", 2, ui::Command(L"Script.Editor.ToggleWildCard"), ui::custom::ToolBarButton::BsDefaultToggle);
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

ui::Size SearchControl::getPreferedSize() const
{
	ui::Size preferedSize = ui::Container::getPreferedSize();
	return ui::Size(ui::dpi96(250), preferedSize.cy);
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

void SearchControl::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
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
	}
}
