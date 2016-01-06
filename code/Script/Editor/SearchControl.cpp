#include "Script/Editor/SearchControl.h"
#include "Script/Editor/SearchEvent.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"

// Resources
#include "Resources/SearchCaseSensitive.h"
#include "Resources/SearchWholeWord.h"
#include "Resources/SearchWildCard.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.SearchControl", SearchControl, ui::Container)

bool SearchControl::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", ui::scaleBySystemDPI(4), ui::scaleBySystemDPI(4))))
		return false;

	m_editSearch = new ui::Edit();
	m_editSearch->create(this, L"", ui::WsWantAllInput);
	m_editSearch->addEventHandler< ui::KeyDownEvent >(this, &SearchControl::eventEditSearchKeyDown);

	m_toolBarMode = new ui::custom::ToolBar();
	m_toolBarMode->create(this);
	m_toolBarMode->addImage(ui::Bitmap::load(c_ResourceSearchCaseSensitive, sizeof(c_ResourceSearchCaseSensitive), L"image"), 1);
	m_toolBarMode->addImage(ui::Bitmap::load(c_ResourceSearchWholeWord, sizeof(c_ResourceSearchWholeWord), L"image"), 1);
	m_toolBarMode->addImage(ui::Bitmap::load(c_ResourceSearchWildCard, sizeof(c_ResourceSearchWildCard), L"image"), 1);
	
	m_toolCaseSensitive = new ui::custom::ToolBarButton(L"Toggle case sensitive search", 0, ui::Command(L"Script.Editor.ToggleCaseSensitive"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolCaseSensitive);

	m_toolWholeWord = new ui::custom::ToolBarButton(L"Toggle whole word search", 1, ui::Command(L"Script.Editor.ToggleWholeWord"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolWholeWord);

	m_toolWildCard = new ui::custom::ToolBarButton(L"Toggle wild-card search", 2, ui::Command(L"Script.Editor.ToggleWildCard"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolBarMode->addItem(m_toolWildCard);

	return true;
}

void SearchControl::setFocus()
{
	m_editSearch->selectAll();
	m_editSearch->setFocus();
}

ui::Size SearchControl::getPreferedSize() const
{
	ui::Size preferedSize = ui::Container::getPreferedSize();
	return ui::Size(ui::scaleBySystemDPI(250), preferedSize.cy);
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
			SearchEvent searchEvent(
				this,
				m_editSearch->getText(),
				m_toolCaseSensitive->isToggled(),
				m_toolWholeWord->isToggled(),
				m_toolWildCard->isToggled()
			);
			raiseEvent(&searchEvent);
		}
	}
}

	}
}
