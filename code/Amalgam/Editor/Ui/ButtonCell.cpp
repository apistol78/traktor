#include "Amalgam/Editor/Ui/ButtonCell.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{

ButtonCell::ButtonCell(
	ui::Bitmap* bitmap,
	int32_t index,
	bool enable,
	int32_t eventId,
	Object* eventItem,
	const ui::Command& eventCommand
)
:	m_bitmap(bitmap)
,	m_index(index)
,	m_enable(enable)
,	m_down(false)
,	m_eventId(eventId)
,	m_eventItem(eventItem)
,	m_eventCommand(eventCommand)
{
}

void ButtonCell::setEnable(bool enable)
{
	m_enable = enable;
}

void ButtonCell::mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	if (m_enable)
		m_down = true;
}

void ButtonCell::mouseUp(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	if (m_enable)
	{
		m_down = false;
		ui::CommandEvent commandEvent(widget, m_eventItem, m_eventCommand);
		widget->raiseEvent(m_eventId, &commandEvent);
	}
}

void ButtonCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
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
