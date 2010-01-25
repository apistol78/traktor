#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarEmbed", ToolBarEmbed, ToolBarItem)

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
	return Size(m_width, imageHeight);
}

void ToolBarEmbed::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight)
{
	Rect rc(at, Size(m_width, imageHeight));
	m_widget->setRect(rc);
}

bool ToolBarEmbed::mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	return false;
}

void ToolBarEmbed::mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent)
{
}

void ToolBarEmbed::buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent)
{
}

void ToolBarEmbed::buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent)
{
}

		}
	}
}
