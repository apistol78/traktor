#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarButton", ToolBarButton, ToolBarItem)

ToolBarButton::ToolBarButton(const std::wstring& text, uint32_t imageIndex, const Command& command, int32_t style)
:	m_text(text)
,	m_command(command)
,	m_imageIndex(imageIndex)
,	m_style(style)
,	m_state(BstNormal)
{
	if ((m_style & BsToggled) == BsToggled)
		setToggled(true);
}

ToolBarButton::ToolBarButton(const std::wstring& text, const Command& command, int32_t style)
:	m_text(text)
,	m_command(command)
,	m_imageIndex(-1)
,	m_style(style)
,	m_state(BstNormal)
{
	if ((m_style & BsToggled) == BsToggled)
		setToggled(true);
}

void ToolBarButton::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ToolBarButton::getText() const
{
	return m_text;
}

void ToolBarButton::setImage(uint32_t imageIndex)
{
	m_imageIndex = imageIndex;
}

uint32_t ToolBarButton::getImage() const
{
	return m_imageIndex;
}

void ToolBarButton::setToggled(bool toggled)
{
	if (toggled)
		m_state |= BstToggled;
	else
		m_state &= ~BstToggled;
}

bool ToolBarButton::isToggled() const
{
	return bool((m_state & BstToggled) == BstToggled);
}

bool ToolBarButton::getToolTip(std::wstring& outToolTip) const
{
	outToolTip = m_text;
	return true;
}

Size ToolBarButton::getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const
{
	int width = 8;
	int height = 8;

	if (m_style & BsIcon)
	{
		width += imageWidth;
		height = max(imageHeight + 8, height);
	}
	if (m_style & BsText)
	{
		Size textExtent = toolBar->getTextExtent(m_text);
		width += textExtent.cx;
		height = max(textExtent.cy + 8, height);
	}
	if ((m_style & (BsIcon | BsText)) == (BsIcon | BsText))
		width += 4;

	return Size(width, height);
}

void ToolBarButton::paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight)
{
	Size size = getSize(toolBar, imageWidth, imageHeight);

	if ((m_state & (BstPushed | BstHover | BstToggled)) != 0)
	{
		if ((m_state & BstPushed) != 0)
		{
			canvas.setForeground(Color4ub(128, 128, 140));
			canvas.setBackground(Color4ub(192, 192, 208));
		}
		else if ((m_state & (BstHover | BstToggled)) != 0)
		{
			canvas.setForeground(Color4ub(128, 128, 140));
			canvas.setBackground(Color4ub(224, 224, 240));
		}
		canvas.fillRect(Rect(
			at,
			size
		));
		canvas.drawRect(Rect(
			at,
			size
		));
	}

	int centerOffsetX = 4;
	if (m_style & BsIcon)
	{
		int centerOffsetY = (size.cy - imageHeight) / 2;
		canvas.drawBitmap(
			at + Size(centerOffsetX, centerOffsetY),
			Point(m_imageIndex * imageWidth, 0),
			Size(imageWidth, imageHeight),
			images,
			BmAlpha
		);
		centerOffsetX += imageWidth + 4;
	}
	if (m_style & BsText)
	{
		Size textExtent = toolBar->getTextExtent(m_text);
		int centerOffsetY = (size.cy - textExtent.cy) / 2;
		canvas.setForeground(Color4ub(64, 64, 70));
		canvas.drawText(
			at + Size(centerOffsetX, centerOffsetY),
			m_text
		);
	}
}

bool ToolBarButton::mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_state |= BstHover;
	return true;
}

void ToolBarButton::mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_state &= ~BstHover;
}

void ToolBarButton::buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_state |= BstPushed;
}

void ToolBarButton::buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent)
{
	m_state &= ~BstPushed;
	if (m_style & BsToggle)
	{
		if (m_state & BstToggled)
			m_state &= ~BstToggled;
		else
			m_state |= BstToggled;
	}

	CommandEvent cmdEvent(toolBar, this, m_command);
	toolBar->raiseEvent(EiClick, &cmdEvent);
}

		}
	}
}
