#ifndef traktor_world_PostProcessStepCell_H
#define traktor_world_PostProcessStepCell_H

#include "Core/Ref.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace world
	{

class PostProcessStepItem;

class PostProcessStepCell : public ui::custom::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	PostProcessStepCell(PostProcessStepItem* item);

	PostProcessStepItem* getItem() const;

	virtual void mouseDown(const ui::Point& position);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< PostProcessStepItem > m_item;
	Ref< ui::Bitmap > m_bitmapStep;
};

	}
}

#endif	// traktor_world_PostProcessStepCell_H
