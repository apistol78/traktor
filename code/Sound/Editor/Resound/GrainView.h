#ifndef traktor_sound_GrainView_H
#define traktor_sound_GrainView_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace sound
	{

class GrainViewCell;
class GrainViewItem;

class GrainView : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(GrainViewItem* item);

	void removeAll();

	GrainViewItem* getSelected() const;

protected:
	virtual void layoutCells(const ui::Rect& rc);

private:
	RefArray< GrainViewCell > m_cells;
};

	}
}

#endif	// traktor_sound_GrainView_H
