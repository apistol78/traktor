#ifndef traktor_amalgam_GraphCell_H
#define traktor_amalgam_GraphCell_H

#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class GraphCell : public ui::custom::AutoWidgetCell
{
public:
	GraphCell(TargetInstance* instance);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_GraphCell_H
