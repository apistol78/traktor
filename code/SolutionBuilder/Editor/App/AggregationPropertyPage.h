#pragma once

#include <Ui/Container.h>
#include <Ui/CheckBox.h>
#include <Ui/Edit.h>
#include <Ui/DropDown.h>
#include <Ui/GridView/GridRowDoubleClickEvent.h>
#include <Ui/GridView/GridView.h>

namespace traktor
{
	namespace sb
	{

class Solution;
class Aggregation;

class AggregationPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(Solution* solution, Aggregation* aggregation);

private:
	Ref< Solution > m_solution;
	Ref< Aggregation > m_aggregation;
	Ref< ui::CheckBox > m_checkEnable;
	Ref< ui::GridView > m_gridDependencies;
	Ref< ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(ui::ButtonClickEvent* event);

	void eventDependencyDoubleClick(ui::GridRowDoubleClickEvent* event);

	void eventClickAdd(ui::ButtonClickEvent* event);

	void eventClickRemove(ui::ButtonClickEvent* event);

	void eventClickAddExternal(ui::ButtonClickEvent* event);
};

	}
}

