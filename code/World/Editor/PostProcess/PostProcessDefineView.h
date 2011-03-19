#ifndef traktor_world_PostProcessDefineView_H
#define traktor_world_PostProcessDefineView_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace world
	{

class PostProcessDefineCell;
class PostProcessDefineItem;

class PostProcessDefineView : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(PostProcessDefineItem* item);

	void removeAll();

protected:
	virtual void layoutCells(const ui::Rect& rc);

private:
	RefArray< PostProcessDefineCell > m_cells;
};

	}
}

#endif	// traktor_world_PostProcessDefineView_H
