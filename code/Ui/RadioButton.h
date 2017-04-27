/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_RadioButton_H
#define traktor_ui_RadioButton_H

#include "Ui/Button.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Radio button.
 * \ingroup UI
 */
class T_DLLCLASS RadioButton : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"", bool checked = false);
	
	void setChecked(bool checked);
	
	bool isChecked() const;
};

	}
}

#endif	// traktor_ui_RadioButton_H
