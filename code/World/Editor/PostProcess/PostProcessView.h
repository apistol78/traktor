#ifndef traktor_world_PostProcessView_H
#define traktor_world_PostProcessView_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace world
	{

class PostProcessStepCell;
class PostProcessStepItem;

class PostProcessView : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(PostProcessStepItem* item);

	void removeAll();

protected:
	virtual void layoutCells(const ui::Rect& rc);

private:
	RefArray< PostProcessStepCell > m_cells;
};

	}
}

#endif	// traktor_world_PostProcessView_H
