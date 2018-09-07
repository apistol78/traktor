/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/MessageBox.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MessageBox", MessageBox, ConfigDialog)

bool MessageBox::create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	if (!ConfigDialog::create(parent, caption, dpi96(200), dpi96(100), Dialog::WsDefaultFixed, new TableLayout(L"100%", L"100%,*", dpi96(4), dpi96(4))))
		return false;

	Ref< Container > ctContent = new Container();
	ctContent->create(this, WsNone, new TableLayout(L"100%", L"100%", 0, 0));

	Ref< Static > staticMessage = new Static();
	staticMessage->create(ctContent, message);

	return true;
}

int MessageBox::show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	MessageBox mb;
	
	if (!mb.create(parent, message, caption, style))
		return DrCancel;

	int result = mb.showModal();

	mb.destroy();

	return result;
}

int MessageBox::show(const std::wstring& message, const std::wstring& caption, int style)
{
	return show(nullptr, message, caption, style);
}

	}
}
