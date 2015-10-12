#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Drawing/Image.h"
#include "Editor/App/NewWorkspaceDialog.h"
#include "I18N/Text.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/PathDialog.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.NewWorkspaceDialog", NewWorkspaceDialog, ui::ConfigDialog)

bool NewWorkspaceDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"EDITOR_NEW_WORKSPACE_TITLE"), 500, 400, ui::ConfigDialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"100%,*", 4, 4)))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &NewWorkspaceDialog::eventDialogClick);

	m_templateList = new ui::custom::PreviewList();
	m_templateList->create(this, ui::WsDoubleBuffer);

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4)))
		return false;

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_NAME"));

	m_editName = new ui::Edit();
	m_editName->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_NAME_DEFAULT"));

	Ref< ui::Static > staticPath = new ui::Static();
	staticPath->create(containerInner, i18n::Text(L"EDITOR_NEW_WORKSPACE_PATH"));

	Ref< ui::Container > containerPath = new ui::Container();
	containerPath->create(containerInner, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 0));

	m_editPath = new ui::Edit();
	m_editPath->create(containerPath, L"");

	Ref< ui::custom::MiniButton > buttonBrowsePath = new ui::custom::MiniButton();
	buttonBrowsePath->create(containerPath, L"...");
	buttonBrowsePath->addEventHandler< ui::ButtonClickEvent >(this, &NewWorkspaceDialog::eventBrowsePathButtonClick);

	// Find templates and add to list.
	RefArray< File > templateFiles;
	FileSystem::getInstance().find(L"$(TRAKTOR_HOME)/res/templates/*.template", templateFiles);

	Ref< ui::custom::PreviewItems > previewItems = new ui::custom::PreviewItems();

	for (RefArray< File >::const_iterator i = templateFiles.begin(); i != templateFiles.end(); ++i)
	{
		Ref< xml::Document > doc = new xml::Document();
		if (!doc->loadFromFile((*i)->getPath()))
			continue;

		Ref< xml::Element > xdescription = doc->getSingle(L"/template/description");
		Ref< xml::Element > ximage = doc->getSingle(L"/template/image");
		if (!xdescription || !ximage)
			continue;

		Path imagePath = FileSystem::getInstance().getAbsolutePath((*i)->getPath().getPathOnly(), ximage->getValue());

		Ref< drawing::Image > image = drawing::Image::load(imagePath);
		if (!image)
			continue;

		Ref< ui::custom::PreviewItem > previewItem = new ui::custom::PreviewItem();
		previewItem->setText(xdescription->getValue());
		previewItem->setImage(new ui::Bitmap(image));
		previewItem->setData(L"FILE", *i);

		previewItems->add(previewItem);
	}

	m_templateList->setItems(previewItems);

	return true;
}

const std::wstring& NewWorkspaceDialog::getWorkspacePath() const
{
	return m_workspacePath;
}

void NewWorkspaceDialog::eventBrowsePathButtonClick(ui::ButtonClickEvent* event)
{
	ui::PathDialog pathDialog;
	Path path;

	pathDialog.create(this, i18n::Text(L"EDITOR_NEW_WORKSPACE_PATH"));
	if (pathDialog.showModal(path) == ui::DrOk)
		m_editPath->setText(path.getPathName());

	pathDialog.destroy();
}

void NewWorkspaceDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if (event->getCommand() == ui::DrOk)
	{
		Ref< ui::custom::PreviewItem > selectedItem = m_templateList->getSelectedItem();
		if (selectedItem)
		{
			Ref< File > file = selectedItem->getData< File >(L"FILE");
			T_FATAL_ASSERT (file);

			std::wstring name = m_editName->getText();
			std::wstring outputPath = Path(m_editPath->getText() + L"/" + name).normalized().getPathName();

			Ref< Environment > env = OS::getInstance().getEnvironment();
			env->set(L"WIZARD_NAME", name);
			env->set(L"WIZARD_OUTPUT_PATH", outputPath);
			env->set(L"WIZARD_TEMPLATE_FILE", file->getPath().getPathName());

			Ref< IProcess > process = OS::getInstance().execute(
#if defined(_WIN64)
				L"$(TRAKTOR_HOME)/bin/latest/win64/releaseshared/Traktor.Run.App $(TRAKTOR_HOME)/bin/template/create-workspace.run",
#elif defined(_WIN32)
				L"$(TRAKTOR_HOME)/bin/latest/win32/releaseshared/Traktor.Run.App $(TRAKTOR_HOME)/bin/template/create-workspace.run",
#elif defined(__APPLE__)
				L"$(TRAKTOR_HOME)/bin/latest/osx/releaseshared/Traktor.Run.App $(TRAKTOR_HOME)/bin/template/create-workspace.run",
#elif defined(__LINUX__)
				L"$(TRAKTOR_HOME)/bin/latest/linux/releaseshared/Traktor.Run.App $(TRAKTOR_HOME)/bin/template/create-workspace.run",
#endif
				file->getPath().getPathOnly(),
				env,
				false,
				true,
				false
			);
			if (process)
			{
				process->wait();

				if (process->exitCode() == 0)
					m_workspacePath = outputPath + L"/" + name + L".workspace";

				process = 0;
			}
			else
				log::error << L"Unable to create workspace; failed to launch process." << Endl;
		}
	}
}

	}
}
