#ifndef traktor_sound_GrainViewCell_H
#define traktor_sound_GrainViewCell_H

#include "Core/Ref.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace sound
	{

class GrainViewItem;

class GrainViewCell : public ui::custom::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	GrainViewCell(GrainViewItem* item);

	GrainViewItem* getItem() const;

	virtual void mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ui::Bitmap > m_bitmapGrain;
	Ref< GrainViewItem > m_item;
};

	}
}

#endif	// traktor_sound_GrainViewCell_H
