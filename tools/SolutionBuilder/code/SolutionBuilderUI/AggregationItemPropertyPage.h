/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef AggregationItemPropertyPage_H
#define AggregationItemPropertyPage_H

#include <Ui/Container.h>
#include <Ui/Edit.h>

class AggregationItem;

class AggregationItemPropertyPage : public traktor::ui::Container
{
public:
	bool create(traktor::ui::Widget* parent);

	void set(AggregationItem* aggregationItem);

private:
	traktor::Ref< AggregationItem > m_aggregationItem;
	traktor::Ref< traktor::ui::Edit > m_editSourceFile;
	traktor::Ref< traktor::ui::Edit > m_editTargetPath;

	void eventEditFocus(traktor::ui::FocusEvent* event);
};

#endif	// AggregationItemPropertyPage_H
