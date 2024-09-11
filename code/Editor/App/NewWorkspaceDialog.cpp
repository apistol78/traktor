/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Drawing/Image.h"
#include "Editor/App/NewWorkspaceDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/PathDialog.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/MiniButton.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.NewWorkspaceDialog", NewWorkspaceDialog, ui::ConfigDialog)

bool NewWorkspaceDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"EDITOR_NEW_WORKSPACE_TITLE"),
		500_ut,
		400_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4_ut, 4_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &NewWorkspaceDialog::eventDialogClick);

	m_templateList = new ui::PreviewList();
	m_templateList->create(this, ui::WsDoubleBuffer);

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 4_ut)))
		return false;

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_NAME"));

	m_editName = new ui::Edit();
	m_editName->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_NAME_DEFAULT"));
	m_editName->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent*) {
		updateWorkspaceFile();
	});

	Ref< ui::Static > staticPath = new ui::Static();
	staticPath->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_PATH"));

	Ref< ui::Container > containerPath = new ui::Container();
	containerPath->create(containerInner, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0_ut, 0_ut));

	m_editPath = new ui::Edit();
	m_editPath->create(containerPath, L"");
	m_editPath->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent*) {
		updateWorkspaceFile();
	});

	Ref< ui::MiniButton > buttonBrowsePath = new ui::MiniButton();
	buttonBrowsePath->create(containerPath, L"...");
	buttonBrowsePath->addEventHandler< ui::ButtonClickEvent >(this, &NewWorkspaceDialog::eventBrowsePathButtonClick);

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerInner, L"");

	m_editWorkspaceFile = new ui::Edit();
	m_editWorkspaceFile->create(containerInner, L"", ui::Edit::WsReadOnly);
	m_editWorkspaceFile->setEnable(false);

	// Find templates and add to list.
	Ref< ui::PreviewItems > previewItems = new ui::PreviewItems();

	RefArray< File > templateFiles = FileSystem::getInstance().find(L"$(TRAKTOR_HOME)/resources/runtime/editor/templates/*.template");
	for (auto templateFile : templateFiles)
	{
		Ref< xml::Document > doc = new xml::Document();
		if (!doc->loadFromFile(templateFile->getPath()))
			continue;

		Ref< xml::Element > xdescription = doc->getSingle(L"/template/description");
		Ref< xml::Element > ximage = doc->getSingle(L"/template/image");
		if (!xdescription || !ximage)
			continue;

		Path imagePath = FileSystem::getInstance().getAbsolutePath(templateFile->getPath().getPathOnly(), ximage->getValue());

		Ref< drawing::Image > image = drawing::Image::load(imagePath);
		if (!image)
			continue;

		Ref< ui::PreviewItem > previewItem = new ui::PreviewItem();
		previewItem->setText(xdescription->getValue());
		previewItem->setImage(new ui::Bitmap(image));
		previewItem->setData(L"FILE", templateFile);
		previewItems->add(previewItem);
	}

	if (previewItems->count() > 0)
	{
		m_templateList->setItems(previewItems);
		m_templateList->getItems()->get(0)->setSelected(true);
	}

	updateWorkspaceFile();
	return true;
}

const std::wstring& NewWorkspaceDialog::getWorkspacePath() const
{
	return m_workspacePath;
}

void NewWorkspaceDialog::updateWorkspaceFile()
{
	std::wstring name = m_editName->getText();
	std::wstring outputPath = Path(m_editPath->getText() + L"/" + toLower(name)).normalized().getPathName();
	m_editWorkspaceFile->setText(outputPath + L"/" + name + L".workspace");
	m_editWorkspaceFile->update();
}

void NewWorkspaceDialog::eventBrowsePathButtonClick(ui::ButtonClickEvent* event)
{
	ui::PathDialog pathDialog;
	Path path;

	pathDialog.create(this, i18n::Text(L"EDITOR_NEW_WORKSPACE_PATH"));
	if (pathDialog.showModal(path) == ui::DialogResult::Ok)
		m_editPath->setText(path.getPathName());

	pathDialog.destroy();
	updateWorkspaceFile();
}

void NewWorkspaceDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if ((ui::DialogResult)event->getCommand().getId() == ui::DialogResult::Ok)
	{
		Ref< ui::PreviewItem > selectedItem = m_templateList->getSelectedItem();
		if (selectedItem)
		{
			Ref< File > file = selectedItem->getData< File >(L"FILE");
			T_FATAL_ASSERT (file);

			std::wstring name = m_editName->getText();
			std::wstring outputPath = Path(m_editPath->getText() + L"/" + toLower(name)).normalized().getPathName();

			Ref< Environment > env = OS::getInstance().getEnvironment();
			env->set(L"WIZARD_NAME", name);
			env->set(L"WIZARD_OUTPUT_PATH", outputPath);
			env->set(L"WIZARD_TEMPLATE_FILE", file->getPath().getPathName());

#if !defined(__DEBUG)
			const std::wstring buildConfigurationProducts = L"releaseshared";
#else
			const std::wstring buildConfigurationProducts = L"debugshared";
#endif
			Ref< IProcess > process = OS::getInstance().execute(
#if defined(_WIN64)
				L"$(TRAKTOR_HOME)/bin/latest/win64/" + buildConfigurationProducts + L"/Traktor.Run.App \"$(TRAKTOR_HOME)/scripts/runtime/editor/templates/create-workspace.run\"",
#elif defined(_WIN32)
				L"$(TRAKTOR_HOME)/bin/latest/win32/" + buildConfigurationProducts + L"/Traktor.Run.App \"$(TRAKTOR_HOME)/scripts/runtime/editor/templates/create-workspace.run\"",
#elif defined(__APPLE__)
				L"$(TRAKTOR_HOME)/bin/latest/osx/" + buildConfigurationProducts + L"/Traktor.Run.App \"$(TRAKTOR_HOME)/scripts/runtime/editor/templates/create-workspace.run\"",
#elif defined(__LINUX__) || defined(__RPI__)
				L"$(TRAKTOR_HOME)/bin/latest/linux/" + buildConfigurationProducts + L"/Traktor.Run.App \"$(TRAKTOR_HOME)/scripts/runtime/editor/templates/create-workspace.run\"",
#endif
				file->getPath().getPathOnly(),
				env,
				OS::EfRedirectStdIO
			);
			if (process)
			{
				PipeReader stdOutReader(
					process->getPipeStream(IProcess::SpStdOut)
				);
				PipeReader stdErrReader(
					process->getPipeStream(IProcess::SpStdErr)
				);

				std::wstring str;
				for (;;)
				{
					Ref< IStream > pipe;
					IProcess::WaitPipeResult result = process->waitPipeStream(100, pipe);
					if (result == IProcess::Ready && pipe != nullptr)
					{
						if (pipe == process->getPipeStream(IProcess::SpStdOut))
						{
							PipeReader::Result result;
							while ((result = stdOutReader.readLine(str)) == PipeReader::RtOk)
								log::info << str << Endl;
						}
						else if (pipe == process->getPipeStream(IProcess::SpStdErr))
						{
							PipeReader::Result result;
							while ((result = stdErrReader.readLine(str)) == PipeReader::RtOk)
								log::error << str << Endl;
						}
					}
					else if (result == IProcess::Terminated)
						break;
				}

				if (process->exitCode() == 0)
					m_workspacePath = outputPath + L"/" + name + L".workspace";

				process = nullptr;
			}
			else
				log::error << L"Unable to create workspace; failed to launch process." << Endl;
		}
	}
}

	}
}
