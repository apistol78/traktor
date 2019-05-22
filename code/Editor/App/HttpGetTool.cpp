#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Editor/App/HttpGetTool.h"
#include "I18N/Text.h"
#include "Net/Url.h"
#include "Net/UrlConnection.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.HttpGetTool", 0, HttpGetTool, IEditorTool)

std::wstring HttpGetTool::getDescription() const
{
	return i18n::Text(L"EDITOR_HTTP_GET");
}

Ref< ui::IBitmap > HttpGetTool::getIcon() const
{
	return 0;
}

bool HttpGetTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool HttpGetTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
{
	ui::InputDialog inputDialog;
	ui::InputDialog::Field f[] =
	{
		ui::InputDialog::Field(
			i18n::Text(L"EDITOR_HTTP_GET_URL")
		)
	};

	inputDialog.create(
		parent,
		i18n::Text(L"EDITOR_HTTP_GET_URL_CAPTION"),
		i18n::Text(L"EDITOR_HTTP_GET_URL_MESSAGE"),
		f,
		sizeof_array(f)
	);
	auto result = inputDialog.showModal();
	inputDialog.destroy();

	if (result != ui::DrOk)
		return true;

	net::Url url(f[0].value);
	if (!url.valid())
	{
		log::error << L"Http GET failed; Invalid URL." << Endl;
		return false;
	}

	Path fileName = url.getFile();

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(
		parent,
		type_name(this),
		i18n::Text(L"EDITOR_HTTP_GET_SAVE_AS"),
		L"All files (*.*);*.*",
		true
	);
	if (saveAsDialog.showModal(fileName) != ui::DrOk)
		return true;

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection)
	{
		log::error << L"Http GET failed; Unable to establish connection to \"" << url.getString() << L"\"." << Endl;
		return false;
	}

	Ref< traktor::IStream > stream = connection->getStream();
	T_ASSERT(stream);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
	{
		log::error << L"Http GET failed; Unable to create file \"" << fileName.getPathName() << L"\"." << Endl;
		return false;
	}

	if (!StreamCopy(file, stream).execute())
	{
		log::error << L"Http GET failed; Failed to download file." << Endl;
		return false;
	}

	log::info << L"Downloaded " << url.getString() << L" as " << fileName.getPathName() << L" successfully." << Endl;
	return true;
}

	}
}
