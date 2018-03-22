/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_ErrorDialog_H
#define traktor_amalgam_ErrorDialog_H

#include "Core/Ref.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class LogList;

		}
	}

	namespace amalgam
	{

class ErrorDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create();

	void addErrorString(const std::wstring& errorString);

private:
	Ref< ui::custom::LogList > m_listLog;

	void eventButtonCopyQuit(ui::ButtonClickEvent* event);

	void eventButtonClickQuit(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_amalgam_ErrorDialog_H
