#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarSeparator", ToolBarSeparator, ToolBarItem)

bool ToolBarSeparator::getToolTip(std::wstring& outToolTip) const
{
	return false;
}

Size ToolBarSeparator::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	return Size(1, imageHeight);
}

void ToolBarSeparator::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight)
{
	canvas.setForeground(Color4ub(160, 160, 160));
	canvas.drawLine(at, at + Size(0, imageHeight));
}

bool ToolBarSeparator::mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent)
{
	return false;
}

void ToolBarSeparator::mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent)
{
}

void ToolBarSeparator::buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent)
{
}

void ToolBarSeparator::buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent)
{
}

		}
	}
}
