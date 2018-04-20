/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "I18N/Text.h"
#include "Script/Editor/ScriptProfilerView.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptProfilerView", ScriptProfilerView, ui::Container)

ScriptProfilerView::ScriptProfilerView(editor::IEditor* editor, IScriptProfiler* scriptProfiler)
:	m_editor(editor)
,	m_scriptProfiler(scriptProfiler)
{
	m_scriptProfiler->addListener(this);
}

ScriptProfilerView::~ScriptProfilerView()
{
	m_scriptProfiler->removeListener(this);
}

bool ScriptProfilerView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_profilerTools = new ui::custom::ToolBar();
	if (!m_profilerTools->create(this))
		return false;

	m_profilerTools->addImage(new ui::StyleBitmap(L"Script.ProfilerClear"), 1);
	m_profilerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_PROFILER_CLEAR_SAMPLES"), 0, ui::Command(L"Script.Editor.ClearProfile")));
	m_profilerTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptProfilerView::eventProfilerToolClick);

	m_profileGrid = new ui::custom::GridView();
	m_profileGrid->create(this, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_FUNCTION"), ui::dpi96(160)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_SCRIPT"), ui::dpi96(180)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_INCLUSIVE_TIME"), ui::dpi96(100)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_EXCLUSIVE_TIME"), ui::dpi96(100)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_INCLUSIVE_PERCENT"), ui::dpi96(80)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_EXCLUSIVE_PERCENT"), ui::dpi96(80)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_COUNT"), ui::dpi96(100)));
	m_profileGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptProfilerView::eventProfileGridDoubleClick);

	return true;
}

void ScriptProfilerView::destroy()
{
	ui::Container::destroy();
}

bool ScriptProfilerView::handleCommand(const ui::Command& command)
{
	if (command == L"Script.Editor.ClearProfile")
	{
		m_profileGrid->removeAllRows();
		m_profile.clear();
		updateProfileGrid();
		return true;
	}
	else
		return false;
}

void ScriptProfilerView::updateProfileGrid()
{
	double totalDuration = 0.0;
	for (std::map< std::pair< Guid, std::wstring >, ProfileEntry >::const_iterator i = m_profile.begin(); i != m_profile.end(); ++i)
		totalDuration += i->second.exclusiveDuration;

	for (std::map< std::pair< Guid, std::wstring >, ProfileEntry >::iterator i = m_profile.begin(); i != m_profile.end(); ++i)
	{
		ProfileEntry& pe = i->second;
		if (!pe.row)
		{
			pe.row = new ui::custom::GridRow();
			pe.row->add(new ui::custom::GridItem(i->first.second));

			pe.row->setData(L"SCRIPT_ID", 0);
			pe.row->setData(L"SCRIPT_LINE", 0);

			if (i->first.first.isNotNull())
			{
				Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(i->first.first);
				if (scriptInstance)
					pe.row->add(new ui::custom::GridItem(scriptInstance->getName()));
				else
					pe.row->add(new ui::custom::GridItem(i->first.first.format()));

				pe.row->setData(L"SCRIPT_ID", new PropertyString(i->first.first.format()));

				size_t p = i->first.second.find(L':');
				if (p != std::wstring::npos)
					pe.row->setData(L"SCRIPT_LINE", new PropertyInteger(parseString< int32_t >(i->first.second.substr(p + 1))));
			}
			else
				pe.row->add(new ui::custom::GridItem(i18n::Text(L"SCRIPT_PROFILER_NATIVE_FUNCTION")));

			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 100.0 / totalDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 100.0 / totalDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.callCount)));

			m_profileGrid->addRow(pe.row);
		}
		else
		{
			pe.row->get(2)->setText(toString(pe.inclusiveDuration * 1000.0, 2));
			pe.row->get(3)->setText(toString(pe.exclusiveDuration * 1000.0, 2));
			pe.row->get(4)->setText(toString(pe.inclusiveDuration * 100.0 / totalDuration, 2));
			pe.row->get(5)->setText(toString(pe.exclusiveDuration * 100.0 / totalDuration, 2));
			pe.row->get(6)->setText(toString(pe.callCount));
		}
	}

	m_profileGrid->setSortColumn(3, true, ui::custom::GridView::SmNumerical);
	m_profileGrid->requestUpdate();
}

void ScriptProfilerView::eventProfilerToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ScriptProfilerView::eventProfileGridDoubleClick(ui::MouseDoubleClickEvent* event)
{
	ui::custom::GridRow* selectedRow = m_profileGrid->getSelectedRow();
	if (selectedRow)
	{
		const PropertyString* scriptIdProp = selectedRow->getData< PropertyString >(L"SCRIPT_ID");
		if (scriptIdProp)
		{
			Guid scriptId = Guid(*scriptIdProp);
			int32_t line = *(selectedRow->getData< PropertyInteger >(L"SCRIPT_LINE"));

			Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(scriptId);
			if (scriptInstance)
			{
				m_editor->openEditor(scriptInstance);

				editor::IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
				if (activeEditorPage)
					activeEditorPage->handleCommand(ui::Command(line, L"Script.Editor.GotoLine"));
			}
		}
	}
}

void ScriptProfilerView::callEnter(const Guid& scriptId, const std::wstring& function)
{
}

void ScriptProfilerView::callLeave(const Guid& scriptId, const std::wstring& function)
{
}

void ScriptProfilerView::callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	ProfileEntry& pe = m_profile[std::make_pair(scriptId, function)];
	pe.callCount += callCount;
	pe.inclusiveDuration += inclusiveDuration;
	pe.exclusiveDuration += exclusiveDuration;
	updateProfileGrid();
}

	}
}
