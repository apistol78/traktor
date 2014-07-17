#ifndef traktor_world_PostProcessDefineCell_H
#define traktor_world_PostProcessDefineCell_H

#include "Core/Ref.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace world
	{

class PostProcessDefineItem;

class PostProcessDefineCell : public ui::custom::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	PostProcessDefineCell(PostProcessDefineItem* item);

	PostProcessDefineItem* getItem() const;

	virtual void mouseDown(const ui::Point& position);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< PostProcessDefineItem > m_item;
	Ref< ui::Bitmap > m_bitmapDefine;
};

	}
}

#endif	// traktor_world_PostProcessDefineCell_H
