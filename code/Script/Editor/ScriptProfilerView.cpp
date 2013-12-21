#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Script/Editor/ScriptProfilerView.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Events/CommandEvent.h"

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
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

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

void ScriptProfilerView::callMeasured(const std::wstring& function, double timeStamp, double inclusiveDuration, double exclusiveDuration)
{
	ProfileEntry& pe = m_profile[function];
	pe.inclusiveDuration += inclusiveDuration;
	pe.exclusiveDuration += exclusiveDuration;
	pe.count++;

	// Calculate total durations.
	double totalInclusiveDuration = 0.0;
	double totalExclusiveDuration = 0.0;
	for (std::map< std::wstring, ProfileEntry >::const_iterator i = m_profile.begin(); i != m_profile.end(); ++i)
	{
		totalInclusiveDuration += i->second.inclusiveDuration;
		totalExclusiveDuration += i->second.exclusiveDuration;
	}

	// Update profile presentation.
	for (std::map< std::wstring, ProfileEntry >::iterator i = m_profile.begin(); i != m_profile.end(); ++i)
	{
		ProfileEntry& pe = i->second;
		if (!pe.row)
		{
			pe.row = new ui::custom::GridRow();
			pe.row->add(new ui::custom::GridItem(function));
			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 1000.0, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.inclusiveDuration * 100.0 / totalInclusiveDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.exclusiveDuration * 100.0 / totalExclusiveDuration, 2)));
			pe.row->add(new ui::custom::GridItem(toString(pe.count)));
			m_profileGrid->addRow(pe.row);
		}
		else
		{
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(1))->setText(toString(pe.inclusiveDuration * 1000.0, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(2))->setText(toString(pe.exclusiveDuration * 1000.0, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(3))->setText(toString(pe.inclusiveDuration * 100.0 / totalInclusiveDuration, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(4))->setText(toString(pe.exclusiveDuration * 100.0 / totalExclusiveDuration, 2));
			checked_type_cast< ui::custom::GridItem*, false >(pe.row->get().at(5))->setText(toString(pe.count));
		}
	}

	m_profileGrid->update();
}

	}
}
