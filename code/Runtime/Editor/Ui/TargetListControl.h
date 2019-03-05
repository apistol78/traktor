#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor
{
	namespace runtime
	{

class TargetInstanceListItem;

/*! \brief
 * \ingroup Runtime
 */
class TargetListControl : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(TargetInstanceListItem* item);

	void removeAll();

private:
	RefArray< TargetInstanceListItem > m_items;

	virtual void layoutCells(const ui::Rect& rc) override final;
};

	}
}

