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

namespace traktor
{
	namespace sb
	{

class AggregationItem;

class AggregationItemPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(AggregationItem* aggregationItem);

private:
	Ref< AggregationItem > m_aggregationItem;
	Ref< ui::Edit > m_editSourceFile;
	Ref< ui::Edit > m_editTargetPath;

	void eventEditFocus(ui::FocusEvent* event);
};

	}
}

#endif	// AggregationItemPropertyPage_H
