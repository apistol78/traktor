/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_DialogWx_H
#define traktor_ui_DialogWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogWx : public WidgetWxImpl< IDialog, wxDialog >
{
public:
	DialogWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon) T_OVERRIDE T_FINAL;
	
	virtual int showModal() T_OVERRIDE T_FINAL;

	virtual void endModal(int result) T_OVERRIDE T_FINAL;

	virtual void setMinSize(const Size& minSize) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_DialogWx_H
