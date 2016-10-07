#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
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
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_FUNCTION"), ui::scaleBySystemDPI(160)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_SCRIPT"), ui::scaleBySystemDPI(180)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_INCLUSIVE_TIME"), ui::scaleBySystemDPI(100)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_EXCLUSIVE_TIME"), ui::scaleBySystemDPI(100)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_INCLUSIVE_PERCENT"), ui::scaleBySystemDPI(80)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_EXCLUSIVE_PERCENT"), ui::scaleBySystemDPI(80)));
	m_profileGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_PROFILER_COLUMN_COUNT"), ui::scaleBySystemDPI(100)));

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

			if (i->first.first.isNotNull())
			{
				Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(i->first.first);
				if (scriptInstance)
					pe.row->add(new ui::custom::GridItem(scriptInstance->getName()));
				else
					pe.row->add(new ui::custom::GridItem(i->first.first.format()));
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
