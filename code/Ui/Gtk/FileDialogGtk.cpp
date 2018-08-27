/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Gtk/FileDialogGtk.h"
#include "Ui/Gtk/Warp.h"
#include "Ui/Itf/IWidget.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

FileDialogGtk::FileDialogGtk(EventSubject* owner)
:	m_owner(owner)
,	m_fileChooser(nullptr)
{
}

FileDialogGtk::~FileDialogGtk()
{
	T_FATAL_ASSERT(m_fileChooser == nullptr);
}

bool FileDialogGtk::create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	Warp* pr = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(pr != nullptr);

	GtkWindow* parentWindow = GTK_WINDOW(gtk_widget_get_toplevel(pr->widget));
	if (parentWindow == nullptr)
	{
		log::error << L"Unable to get toplevel window from parent widget." << Endl;
		return false;
	}

	m_fileChooser = gtk_file_chooser_dialog_new(
		wstombs(title).c_str(),
		parentWindow,
		save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
		("Cancel"), GTK_RESPONSE_CANCEL,
		("Open"), GTK_RESPONSE_ACCEPT,
		nullptr
	);
	if (m_fileChooser == nullptr)
	{
		log::error << L"Unable to create GTK file chooser dialog." << Endl;
		return false;
	}

	return true;
}

void FileDialogGtk::destroy()
{
	if (m_fileChooser)
	{
		gtk_widget_destroy(m_fileChooser);
		m_fileChooser = 0;
	}
}

int FileDialogGtk::showModal(Path& outPath)
{
	T_ASSERT (m_fileChooser);

	gint res = gtk_dialog_run(GTK_DIALOG(m_fileChooser));
	if (res != GTK_RESPONSE_ACCEPT)
		return DrCancel;

	char* fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_fileChooser));
	if (fileName == nullptr)
		return DrCancel;

	outPath = mbstows(fileName);
	return DrOk;
}

int FileDialogGtk::showModal(std::vector< Path >& outPaths)
{
	return DrCancel;
}

	}
}
