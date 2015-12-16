#include "Amalgam/Editor/Ui/ProgressCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"

namespace traktor
{
	namespace amalgam
	{

ProgressCell::ProgressCell()
:	m_progress(-1)
{
}

void ProgressCell::setText(const std::wstring& text)
{
	m_text = text;
}

void ProgressCell::setProgress(int32_t progress)
{
	m_progress = progress;
	if (m_progress < 0)
		m_progress = 0;
	else if (m_progress > 100)
		m_progress = 100;
}

void ProgressCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	if (m_progress >= 0)
	{
		canvas.setBackground(Color4ub(255, 255, 255, 255));
		canvas.fillRect(rect);

		int32_t x = (rect.getWidth() * m_progress) / 100;
		if (x > 0)
		{
			ui::Rect rect2 = rect;
			rect2.right = rect2.left + x;

			canvas.setBackground(Color4ub(0, 153, 0, 255));
			canvas.fillRect(rect2);
		}
	}
	if (!m_text.empty())
	{
		ui::Rect rect2 = rect; rect2.left += 2;
		canvas.drawText(rect2, m_text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
