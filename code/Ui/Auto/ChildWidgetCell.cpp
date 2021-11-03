#include "Core/Misc/SafeDestroy.h"
#include "Ui/Auto/ChildWidgetCell.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ChildWidgetCell", ChildWidgetCell, AutoWidgetCell)

ChildWidgetCell::ChildWidgetCell(Widget* child)
:	m_child(child)
{
}

ChildWidgetCell::~ChildWidgetCell()
{
	safeDestroy(m_child);
}

void ChildWidgetCell::placeCells(AutoWidget* widget, const Rect& rect)
{
	AutoWidgetCell::placeCells(widget, rect);
	m_child->setRect(getClientRect());
}

Widget* ChildWidgetCell::getChild() const
{
	return m_child;
}

	}
}
