#pragma once

#include "Ui/Container.h"
#include "Ui/Edit.h"

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

