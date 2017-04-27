/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FileDialogWx_H
#define traktor_ui_FileDialogWx_H

#include "Ui/Itf/IFileDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class FileDialogWx : public IFileDialog
{
public:
	FileDialogWx(EventSubject* owner);

	virtual ~FileDialogWx();

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save);

	virtual void destroy();

	virtual int showModal(Path& outPath);

	virtual int showModal(std::vector< Path >& outPaths);

private:
	EventSubject* m_owner;
	wxFileDialog* m_fileDialog;
};

	}
}

#endif	// traktor_ui_FileDialogWx_H
