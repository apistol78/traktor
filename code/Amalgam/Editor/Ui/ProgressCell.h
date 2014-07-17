#ifndef traktor_amalgam_ProgressCell_H
#define traktor_amalgam_ProgressCell_H

#include "Ui/Bitmap.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class ProgressCell : public ui::custom::AutoWidgetCell
{
public:
	ProgressCell();

	void setText(const std::wstring& text);

	void setProgress(int32_t progress);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ui::Bitmap > m_imageProgressBar;
	std::wstring m_text;
	int32_t m_progress;
};

	}
}

#endif	// traktor_amalgam_ProgressCell_H
