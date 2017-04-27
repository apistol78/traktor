/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_RadioButtonWin32_H
#define traktor_ui_RadioButtonWin32_H

#include "Ui/Itf/IRadioButton.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class RadioButtonWin32 : public WidgetWin32Impl< IRadioButton >
{
public:
	RadioButtonWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;
};

	}
}

#endif	// traktor_ui_RadioButtonWin32_H
