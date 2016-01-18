#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

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

void ToolBarSeparator::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	canvas.setForeground(ss->getColor(toolBar, L"item-color-seperator"));
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
