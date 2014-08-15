#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Script/Editor/ScriptProfilerView.h"
#include "Ui/Bitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

// Resources
#include "Resources/Debug.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptProfilerView", ScriptProfilerView, ui::Container)

ScriptProfilerView::ScriptProfilerView(IScriptProfiler* scriptProfiler)
:	m_scriptProfiler(scriptProfiler)
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

	m_profilerTools->addImage(ui::Bitmap::load(c_ResourceDebug, sizeof(c_ResourceDebug), L"png"), 4);
	m_profilerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_CLEAR_PROFILE"), 1, ui::Command(L"Script.Editor.ClearProfile")));
	m_profilerTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptProfilerView::eventProfilerToolClick);

	m_profileGrid = new ui::custom::GridView();
	m_profileGrid->create(this, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Function", 120));
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Inc. (ms)", 100));
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Exc. (ms)", 100));
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Inc. (%)", 80));
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Exc. (%)", 80));
	m_profileGrid->addColumn(new ui::custom::GridColumn(L"Count", 100));

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
	for (std::map< std::wstring, ProfileEntry >::const_iterator i = m_profile.begin(); i != m_profile.end(); ++i)
		totalDuration += i->second.exclusiveDuration;

	for (std::map< std::wstring, ProfileEntry >::iterator i = m_profile.begin(); i != m_profile.end(); ++i)
	{
		ProfileEntry& pe = i->second;
		if (!pe.row)
		{
			pe.row = new ui::custom::GridRow();
			pe.row->add(new ui::custom::GridItem(i->first));
			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 100.0 / totalDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 100.0 / totalDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.callCount)));
			m_profileGrid->addRow(pe.row);
		}
		else
		{
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(1))->setText(toString(pe.inclusiveDuration * 1000.0, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(2))->setText(toString(pe.exclusiveDuration * 1000.0, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(3))->setText(toString(pe.inclusiveDuration * 100.0 / totalDuration, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(4))->setText(toString(pe.exclusiveDuration * 100.0 / totalDuration, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(5))->setText(toString(pe.callCount));
		}
	}

	m_profileGrid->setSortColumn(2, true, ui::custom::GridView::SmNumerical);
	m_profileGrid->update();
}

void ScriptProfilerView::eventProfilerToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ScriptProfilerView::callMeasured(const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	ProfileEntry& pe = m_profile[function];
	pe.callCount += callCount;
	pe.inclusiveDuration += inclusiveDuration;
	pe.exclusiveDuration += exclusiveDuration;
	updateProfileGrid();
}

	}
}
