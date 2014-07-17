#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "World/Editor/PostProcess//PostProcessStepCell.h"
#include "World/Editor/PostProcess//PostProcessStepItem.h"

// Resources
#include "Resources/PostProcessStep.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepCell", PostProcessStepCell, ui::custom::AutoWidgetCell)

PostProcessStepCell::PostProcessStepCell(PostProcessStepItem* item)
:	m_item(item)
{
	m_bitmapStep = ui::Bitmap::load(c_ResourcePostProcessStep, sizeof(c_ResourcePostProcessStep), L"png");
}

PostProcessStepItem* PostProcessStepCell::getItem() const
{
	return m_item;
}

void PostProcessStepCell::mouseDown(const ui::Point& position)
{
	ui::SelectionChangeEvent selectionChange(getWidget(), m_item);
	getWidget()->raiseEvent(&selectionChange);
}

void PostProcessStepCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	bool focus = bool(getWidget()->getFocusCell() == this);

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(0, focus ? 32 : 0),
		ui::Size(128, 32),
		m_bitmapStep,
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
		m_bitmapStep,
		ui::BmAlpha
	);

	std::wstring text = m_item->getText();
	if (!text.empty())
	{
		ui::Rect textRect = rect;
		textRect.left += 36;

		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawText(textRect, text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
