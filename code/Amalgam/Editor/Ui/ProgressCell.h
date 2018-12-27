/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ProgressCell_H
#define traktor_amalgam_ProgressCell_H

#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
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

#endif	// traktor_amalgam_ProgressCell_H
