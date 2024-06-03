/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Editor/LogView.h"
#include "I18N/Text.h"
#include "Ui/Edit.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/LogActivateEvent.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::editor
{
	namespace
	{

class LogListTarget : public ILogTarget
{
public:
	LogListTarget(ui::LogList* logList)
	:	m_logList(logList)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		m_logList->add(threadId, (ui::LogList::LogLevel)(1 << level), str);
	}

private:
	ui::LogList* m_logList;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.LogView", LogView, ui::Container)

LogView::LogView(IEditor* editor)
:	m_editor(editor)
{
}

bool LogView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_toolToggleInfo = new ui::ToolBarButton(
		i18n::Text(L"LOG_VIEW_INFO"),
		2,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleWarning = new ui::ToolBarButton(
		i18n::Text(L"LOG_VIEW_WARNING"),
		3,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::ToolBarButton::BsDefaultToggled
	);

	m_toolToggleError = new ui::ToolBarButton(
		i18n::Text(L"LOG_VIEW_ERROR"),
		1,
		ui::Command(L"Editor.Log.ToggleLevel"),
		ui::ToolBarButton::BsDefaultToggled
	);

	m_toolFilter = new ui::ToolBar();
	m_toolFilter->create(this);
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Copy"));
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Error"));
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Info"));
	m_toolFilter->addImage(new ui::StyleBitmap(L"Editor.Log.Warning"));
	m_toolFilter->addItem(m_toolToggleInfo);
	m_toolFilter->addItem(m_toolToggleWarning);
	m_toolFilter->addItem(m_toolToggleError);
	m_toolFilter->addItem(new ui::ToolBarSeparator());
	m_toolFilter->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), 0, ui::Command(L"Editor.Log.Copy")));
	m_toolFilter->addItem(new ui::ToolBarSeparator());
	
	m_editFind = new ui::Edit();
	m_editFind->create(m_toolFilter, L"", ui::WsNone);
	m_editFind->addEventHandler< ui::KeyEvent >(this, &LogView::eventFindKey);
	m_toolFilter->addItem(new ui::ToolBarEmbed(m_editFind, 130_ut));

	m_toolFilter->addEventHandler< ui::ToolBarButtonClickEvent >(this, &LogView::eventToolClick);

	m_log = new ui::LogList();
	m_log->create(this, ui::WsNone, this);
	m_log->addEventHandler< ui::MouseButtonDownEvent >(this, &LogView::eventButtonDown);
	m_log->addEventHandler< ui::LogActivateEvent >(this, &LogView::eventLogActivate);

	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const ui::Unit fontSize = m_log->getFont().getSize();
	m_log->setFont(ui::Font(font, fontSize));

	m_popup = new ui::Menu();
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.Copy"), i18n::Text(L"LOG_COPY")));
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.CopyFiltered"), i18n::Text(L"LOG_COPY_FILTERED")));
	m_popup->add(new ui::MenuItem(L"-"));
	m_popup->add(new ui::MenuItem(ui::Command(L"Editor.Log.Clear"), i18n::Text(L"LOG_CLEAR_ALL")));

	m_logTarget = new LogListTarget(m_log);
	return true;
}

bool LogView::haveWarnings() const
{
	return m_log->countLog(ui::LogList::LvWarning) != 0;
}

bool LogView::haveErrors() const
{
	return m_log->countLog(ui::LogList::LvError) != 0;
}

void LogView::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Editor.Log.ToggleLevel")
	{
		m_log->setFilter(
			(m_toolToggleInfo->isToggled() ? ui::LogList::LvInfo : 0) |
			(m_toolToggleWarning->isToggled() ? ui::LogList::LvWarning : 0) |
			(m_toolToggleError->isToggled() ? ui::LogList::LvError : 0)
		);
	}
	else if (cmd == L"Editor.Log.Copy")
		m_log->copyLog();
}

void LogView::eventFindKey(ui::KeyEvent* event)
{
	const std::wstring needle = toLower(m_editFind->getText());
	if (!needle.empty())
	{
		AlignedVector< int32_t > lines;
		m_log->forEachFilteredLine([&](int32_t line,  const std::wstring& text) {
			if (toLower(text).find(needle) != text.npos)
				lines.push_back(line);
		});
		if (!lines.empty())
		{
			m_log->selectLine(lines.front());
			m_log->showLine(lines.front());
		}
	}
}

void LogView::eventButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const ui::MenuItem* selected = m_popup->showModal(m_log, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Editor.Log.Copy")
		m_log->copyLog();
	else if (selected->getCommand() == L"Editor.Log.CopyFiltered")
		m_log->copyLog(m_log->getFilter());
	else if (selected->getCommand() == L"Editor.Log.Clear")
		m_log->removeAll();
}

void LogView::eventLogActivate(ui::LogActivateEvent* event)
{
	if (m_editor->getSourceDatabase() == nullptr)
		return;

	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(event->getSymbolId());
	if (instance)
		m_editor->openEditor(instance);
	else
	{
		RefArray< db::Instance > instances;
		db::recursiveFindChildInstances(
			m_editor->getSourceDatabase()->getRootGroup(),
			db::FindInstanceAll(),
			instances
		);
		for (auto instance : instances)
		{
			auto object = instance->getObject();
			if (!object)
				continue;

			Ref< Reflection > r = Reflection::create(object);
			if (!r)
				continue;

			RefArray< RfmPrimitiveGuid > members;
			r->findMembers(RfpMemberType(type_of< RfmPrimitiveGuid >()), (RefArray< ReflectionMember >&)members);
			for (auto member : members)
			{
				if (member->get() == event->getSymbolId())
				{
					m_editor->openEditor(instance);
					return;
				}
			}
		}
	}
}

bool LogView::lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const
{
	if (m_editor->getSourceDatabase() == nullptr)
		return false;

	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(symbolId);
	if (!instance)
		return false;
	outSymbol = instance->getPath();
	return true;
}

}
