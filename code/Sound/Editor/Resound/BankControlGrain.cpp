#include "I18N/Format.h"
#include "Sound/Editor/Resound/BankControlGrain.h"
#include "Ui/Canvas.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidget.h"

// Resources
#include "Resources/Grain.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankControlGrain", BankControlGrain, ui::custom::AutoWidgetCell)

BankControlGrain::BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, int32_t image)
:	m_parent(parent)
,	m_grain(grain)
,	m_text(text)
,	m_image(image)
{
	m_bitmapGrain = ui::Bitmap::load(c_ResourceGrain, sizeof(c_ResourceGrain), L"png");
}

BankControlGrain* BankControlGrain::getParent() const
{
	return m_parent;
}

IGrainData* BankControlGrain::getGrain() const
{
	return m_grain;
}

int32_t BankControlGrain::getImage() const
{
	return m_image;
}

std::wstring BankControlGrain::getText() const
{
	return m_text;
}

void BankControlGrain::mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position)
{
	ui::CommandEvent commandEvent(widget, this);
	widget->raiseEvent(ui::EiSelectionChange, &commandEvent);
}

void BankControlGrain::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	bool focus = bool(widget->getFocusCell() == this);

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(0, focus ? 32 : 0),
		ui::Size(128, 32),
		m_bitmapGrain,
		ui::BmAlpha
	);

	canvas.drawBitmap(
		rect.getTopLeft(),
		ui::Point(
			(m_image % 4) * 32,
			64 + (m_image / 4) * 32
		),
		ui::Size(32, 32),
		m_bitmapGrain,
		ui::BmAlpha
	);

	if (!m_text.empty())
	{
		ui::Rect textRect = rect;
		textRect.left += 36;

		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawText(textRect, m_text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
