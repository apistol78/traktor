#include "Sound/Editor/Resound/GrainViewCell.h"
#include "Sound/Editor/Resound/GrainViewItem.h"
#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"

// Resources
#include "Resources/Grain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainViewCell", GrainViewCell, ui::custom::AutoWidgetCell)

GrainViewCell::GrainViewCell(GrainViewItem* item)
:	m_item(item)
{
	m_bitmapGrain = ui::Bitmap::load(c_ResourceGrain, sizeof(c_ResourceGrain), L"png");
}

GrainViewItem* GrainViewCell::getItem() const
{
	return m_item;
}

void GrainViewCell::mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	ui::CommandEvent commandEvent(widget, m_item);
	widget->raiseEvent(ui::EiSelectionChange, &commandEvent);
}

void GrainViewCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Size& offset)
{
	ui::Rect rect = getRect().offset(offset);

	bool focus = bool(widget->getFocusCell() == this);

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(0, focus ? 32 : 0),
		ui::Size(128, 32),
		m_bitmapGrain,
		ui::BmAlpha
	);

	int32_t image = m_item->getImage();

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(
			(image % 4) * 32,
			64 + (image / 4) * 32
		),
		ui::Size(32, 32),
		m_bitmapGrain,
		ui::BmAlpha
	);

	std::wstring text = m_item->getText();
	if (!text.empty())
	{
		ui::Rect textRect = rect;
		textRect.left += 36;

		canvas.setForeground(Color(0, 0, 0));
		canvas.drawText(textRect, text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
