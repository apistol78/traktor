/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef AggregationPropertyPage_H
#define AggregationPropertyPage_H

#include <Ui/Container.h>
#include <Ui/CheckBox.h>
#include <Ui/Edit.h>
#include <Ui/ListView.h>
#include <Ui/DropDown.h>

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
	Ref< ui::ListView > m_listDependencies;
	Ref< ui::DropDown > m_dropAvailable;

	void updateDependencyList();

	void eventEnableClick(ui::ButtonClickEvent* event);

	void eventDependencyDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventClickAdd(ui::ButtonClickEvent* event);

	void eventClickRemove(ui::ButtonClickEvent* event);

	void eventClickAddExternal(ui::ButtonClickEvent* event);
};

	}
}

#endif	// AggregationPropertyPage_H
