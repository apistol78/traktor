/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/MessageBoxWx.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Enums.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

MessageBoxWx::MessageBoxWx(EventSubject* owner)
{
}

bool MessageBoxWx::create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	int wxStyle = 0;

	if (style & MbOk)
		wxStyle |= wxOK;
	if (style & MbCancel)
		wxStyle |= wxCANCEL;
	if (style & MbYesNo)
		wxStyle |= wxYES_NO;
	if (style & MbNoDefault)
		wxStyle |= wxNO_DEFAULT;
	if (style & MbIconExclamation)
		wxStyle |= wxICON_EXCLAMATION;
	if (style & MbIconHand)
		wxStyle |= wxICON_HAND;
	if (style & MbIconError)
		wxStyle |= wxICON_ERROR;
	if (style & MbIconQuestion)
		wxStyle |= wxICON_QUESTION;
	if (style & MbIconInformation)
		wxStyle |= wxICON_INFORMATION;
	if (style & MbStayOnTop)
		wxStyle |= wxSTAY_ON_TOP;
	
	m_messageBox = new wxMessageDialog(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		wstots(message).c_str(),
		wstots(caption).c_str(),
		wxStyle
	);

	return true;
}

void MessageBoxWx::destroy()
{
	T_ASSERT (m_messageBox);
	m_messageBox->Destroy();
	m_messageBox = 0;
}

int MessageBoxWx::showModal()
{
	int result = m_messageBox->ShowModal();
	if (result == wxID_OK)
		return DrOk;
	else if (result == wxID_YES)
		return DrYes;
	return DrCancel;
}

	}
}
