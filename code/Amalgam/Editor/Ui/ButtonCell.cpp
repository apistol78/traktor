#include "Amalgam/Editor/Ui/ButtonCell.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ButtonCell", ButtonCell, ui::custom::AutoWidgetCell)

ButtonCell::ButtonCell(
	ui::Bitmap* bitmap,
	int32_t index,
	const ui::Command& command
)
:	m_bitmap(bitmap)
,	m_index(index)
,	m_command(command)
,	m_enable(true)
,	m_down(false)
{
}

void ButtonCell::setEnable(bool enable)
{
	m_enable = enable;
}

void ButtonCell::mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)
{
	if (m_enable)
		m_down = true;
}

void ButtonCell::mouseUp(ui::MouseButtonUpEvent* event, const ui::Point& position)
{
	if (m_enable)
	{
		m_down = false;

		ui::ButtonClickEvent clickEvent(this, m_command);
		raiseEvent(&clickEvent);
	}
}

void ButtonCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	ui::Size bitmapSize = m_bitmap->getSize();
	ui::Size glyphSize(bitmapSize.cx / 4, bitmapSize.cy / 2);

	ui::Point position = rect.getTopLeft();
	position.y += (rect.getHeight() - glyphSize.cy) / 2;

	if (m_down)
	{
		position.x += 1;
		position.y += 1;
	}

	canvas.drawBitmap(
		position,
		ui::Point(m_index * glyphSize.cx, (m_enable ? 0 : 1) * glyphSize.cy),
		glyphSize,
		m_bitmap,
		ui::BmAlpha
	);
}

	}
}
