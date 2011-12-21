#ifndef AggregationPropertyPage_H
#define AggregationPropertyPage_H

#include <Ui/Container.h>
#include <Ui/CheckBox.h>
#include <Ui/Edit.h>
#include <Ui/ListView.h>
#include <Ui/DropDown.h>

class Solution;
class Aggregation;

class AggregationPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(Solution* solution, Aggregation* aggregation);

private:
	traktor::Ref< Solution > m_solution;
	traktor::Ref< Aggregation > m_aggregation;
	traktor::Ref< traktor::ui::CheckBox > m_checkEnable;
	traktor::Ref< traktor::ui::ListView > m_listDependencies;
	traktor::Ref< traktor::ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(traktor::ui::Event* event);

	void eventDependencyDoubleClick(traktor::ui::Event* event);

	void eventClickAdd(traktor::ui::Event* event);

	void eventClickRemove(traktor::ui::Event* event);

	void eventClickAddExternal(traktor::ui::Event* event);
};

#endif	// AggregationPropertyPage_H
