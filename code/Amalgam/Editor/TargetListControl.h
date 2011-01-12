#ifndef traktor_amalgam_TargetListControl_H
#define traktor_amalgam_TargetListControl_H

#include "Core/Ref.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

class TargetCell;
class TargetInstance;

class TargetListControl : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(TargetInstance* instance);

	void removeAll();

	void addPlayEventHandler(ui::EventHandler* eventHandler);

	void addStopEventHandler(ui::EventHandler* eventHandler);

private:
	Ref< ui::Bitmap > m_bitmapTargetControl;
	RefArray< TargetCell > m_cells;

	virtual void layoutCells(const ui::Rect& rc);
};

	}
}

#endif	// traktor_amalgam_TargetListControl_H
