#pragma once

#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class ProgressCell : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	ProgressCell();

	void setText(const std::wstring& text);

	void setProgress(int32_t progress);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	std::wstring m_text;
	int32_t m_progress;
};

	}
}

