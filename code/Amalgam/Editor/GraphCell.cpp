#include "Amalgam/Editor/GraphCell.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace amalgam
	{

GraphCell::GraphCell(TargetInstance* instance)
:	m_instance(instance)
{
}

void GraphCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	canvas.setBackground(Color4ub(74, 92, 61));
	canvas.fillRect(rect);

	canvas.setForeground(Color4ub(157, 186, 41));
	canvas.drawLine(rect.left + 1, rect.getCenter().y, rect.right - 2, rect.getCenter().y);
}

	}
}
