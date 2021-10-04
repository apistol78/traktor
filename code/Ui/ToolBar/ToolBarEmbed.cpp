#include "Core/Math/MathUtils.h"
#include "Ui/Widget.h"
#include "Ui/ToolBar/ToolBarEmbed.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarEmbed", ToolBarEmbed, ToolBarItem)

ToolBarEmbed::ToolBarEmbed(Widget* widget, int width)
:	m_widget(widget)
,	m_width(width)
{
}

bool ToolBarEmbed::getToolTip(std::wstring& outToolTip) const
{
	return false;
}

Size ToolBarEmbed::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	Size preferedSize = m_widget->getPreferredSize(Size(m_width, imageHeight));
	return Size(
		max(preferedSize.cx, m_width),
		max(preferedSize.cy, imageHeight)
	);
}

void ToolBarEmbed::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight)
{
	Rect rc(at, getSize(toolBar, imageWidth, imageHeight));
	m_widget->setRect(rc);
}

bool ToolBarEmbed::mouseEnter(ToolBar* toolBar)
{
	return false;
}

void ToolBarEmbed::mouseLeave(ToolBar* toolBar)
{
}

void ToolBarEmbed::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
}

void ToolBarEmbed::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

	}
}
