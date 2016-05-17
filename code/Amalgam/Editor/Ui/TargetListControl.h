#ifndef traktor_amalgam_TargetListControl_H
#define traktor_amalgam_TargetListControl_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstanceListItem;

class TargetListControl : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(TargetInstanceListItem* item);

	void removeAll();

private:
	RefArray< TargetInstanceListItem > m_items;

	virtual void layoutCells(const ui::Rect& rc) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_TargetListControl_H
