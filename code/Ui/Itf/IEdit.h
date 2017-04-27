/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IEdit_H
#define traktor_ui_IEdit_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Edit interface.
 * \ingroup UI
 */
class IEdit : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual void setSelection(int from, int to) = 0;

	virtual void getSelection(int& outFrom, int& outTo) const = 0;

	virtual void selectAll() = 0;

	virtual void setBorderColor(const Color4ub& borderColor) = 0;
};

	}
}

#endif	// traktor_ui_IEdit_H
