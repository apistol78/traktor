/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/FileDialog.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IFileDialog.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FileDialog", FileDialog, EventSubject)

FileDialog::FileDialog()
:	m_fileDialog(0)
{
}

FileDialog::~FileDialog()
{
	T_ASSERT_M (!m_fileDialog, L"FileDialog not destroyed");
}

bool FileDialog::create(Widget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	m_fileDialog = Application::getInstance()->getWidgetFactory()->createFileDialog(this);
	if (!m_fileDialog)
	{
		log::error << L"Failed to create native widget peer (FileDialog)" << Endl;
		return false;
	}

	if (!m_fileDialog->create(parent ? parent->getIWidget() : 0, title, filters, save))
		return false;

	return true;
}

void FileDialog::destroy()
{
	if (m_fileDialog)
	{
		m_fileDialog->destroy();
		m_fileDialog = 0;
	}
}

int FileDialog::showModal(Path& outPath)
{
	T_ASSERT (m_fileDialog);
	return m_fileDialog->showModal(outPath);
}

int FileDialog::showModal(std::vector< Path >& outPaths)
{
	T_ASSERT (m_fileDialog);
	return m_fileDialog->showModal(outPaths);
}

	}
}
