#ifndef traktor_amalgam_ProgressCell_H
#define traktor_amalgam_ProgressCell_H

#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class ProgressCell : public ui::custom::AutoWidgetCell
{
public:
	ProgressCell();

	void setText(const std::wstring& text);

	void setProgress(int32_t progress);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) T_OVERRIDE T_FINAL;

private:
	std::wstring m_text;
	int32_t m_progress;
};

	}
}

#endif	// traktor_amalgam_ProgressCell_H
