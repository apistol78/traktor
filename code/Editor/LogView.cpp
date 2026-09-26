/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallSet.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
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
#include "Editor/IEditorPage.h"
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

/*! Recursively check if object, or any child object, has a reflected guid member matching objectId. */
bool containObject(const ISerializable* object, const Guid& objectId, SmallSet< const ISerializable* >& visited)
{
	if (!visited.insert(object))
		return false;

	Ref< Reflection > r = Reflection::create(object);
	if (!r)
		return false;

	RefArray< RfmPrimitiveGuid > guidMembers;
	r->findMembers(RfpMemberType(type_of< RfmPrimitiveGuid >()), (RefArray< ReflectionMember >&)guidMembers);
	for (auto guidMember : guidMembers)
	{
		if (guidMember->get() == objectId)
			return true;
	}

	RefArray< RfmObject > objectMembers;
	r->findMembers(RfpMemberType(type_of< RfmObject >()), (RefArray< ReflectionMember >&)objectMembers);
	for (auto objectMember : objectMembers)
	{
		const ISerializable* childObject = objectMember->get();
		if (childObject && containObject(childObject, objectId, visited))
			return true;
	}

	return false;
}

Ref< db::Instance > findOwnerInstance(const RefArray< db::Instance >& instances, const Guid& objectId)
{
	for (auto instance : instances)
	{
		Ref< ISerializable > object = instance->getObject();
		if (!object)
			continue;

		SmallSet< const ISerializable* > visited;
		if (containObject(object, objectId, visited))
			return instance;
	}
	return nullptr;
}

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
	return m_log->countUnrenderedLog(ui::LogList::LvWarning) != 0;
}

bool LogView::haveErrors() const
{
	return m_log->countUnrenderedLog(ui::LogList::LvError) != 0;
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
	db::Database* database = m_editor->getSourceDatabase();
	if (database == nullptr)
		return;

	// Symbols which isn't an instance are assumed to identify an object
	// inside an instance, such as a node in a shader graph.
	RefArray< db::Instance > symbolInstances;
	AlignedVector< Guid > objectIds;
	for (const auto& symbolId : event->getSymbolIds())
	{
		Ref< db::Instance > instance = database->getInstance(symbolId);
		if (instance)
			symbolInstances.push_back(instance);
		else
			objectIds.push_back(symbolId);
	}

	// Objects are more specific than instances so they take precedence; open
	// owning instance and let the editor page focus on the object.
	RefArray< db::Instance > allInstances;
	for (const auto& objectId : objectIds)
	{
		// Search instances mentioned in the same log line first since
		// object ids can be duplicated in copies of an instance.
		Ref< db::Instance > ownerInstance = findOwnerInstance(symbolInstances, objectId);
		if (!ownerInstance)
		{
			if (allInstances.empty())
				db::recursiveFindChildInstances(database->getRootGroup(), db::FindInstanceAll(), allInstances);
			ownerInstance = findOwnerInstance(allInstances, objectId);
		}
		if (!ownerInstance)
			continue;

		if (m_editor->openEditor(ownerInstance))
		{
			IEditorPage* editorPage = m_editor->getActiveEditorPage();
			if (editorPage)
				editorPage->handleCommand(ui::Command(L"Editor.FocusObject", new PropertyString(objectId.format())));
		}
		return;
	}

	if (!symbolInstances.empty())
		m_editor->openEditor(symbolInstances.front());
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
