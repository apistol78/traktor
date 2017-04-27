/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ICheckBox_H
#define traktor_ui_ICheckBox_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief CheckBox interface.
 * \ingroup UI
 */
class ICheckBox : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, bool checked) = 0;

	virtual void setChecked(bool checked) = 0;

	virtual bool isChecked() const = 0;
};

	}
}

#endif	// traktor_ui_ICheckBox_H
