#include "Editor/App/ConvertDatabaseTool.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Ui/FileDialog.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "I18N/Text.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

void recursiveConvertInstances(db::Group* targetGroup, db::Group* sourceGroup, ui::custom::BackgroundWorkerStatus& status)
{
	for (RefArray< db::Instance >::iterator i = sourceGroup->getBeginChildInstance(); i != sourceGroup->getEndChildInstance(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT (sourceInstance);

		log::info << L"Converting \"" << sourceInstance->getName() << L"\"..." << Endl;
		status.notify(0, sourceInstance->getName());

		Ref< ISerializable > sourceObject = sourceInstance->getObject();
		if (!sourceObject)
		{
			log::error << L"Failed, unable to get source object" << Endl;
			continue;
		}

		Guid sourceGuid = sourceInstance->getGuid();
		Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), db::CifReplaceExisting, &sourceGuid);
		if (!targetInstance)
		{
			log::error << L"Failed, unable to create target instance" << Endl;
			continue;
		}

		targetInstance->setObject(sourceObject);

		std::vector< std::wstring > dataNames;
		sourceInstance->getDataNames(dataNames);

		for (std::vector< std::wstring >::iterator j = dataNames.begin(); j != dataNames.end(); ++j)
		{
			log::info << L"\t\"" << *j << L"\"..." << Endl;

			Ref< IStream > sourceStream = sourceInstance->readData(*j);
			if (!sourceStream)
			{
				log::error << L"Failed, unable to open source stream" << Endl;
				continue;
			}

			Ref< IStream > targetStream = targetInstance->writeData(*j);
			if (!targetStream)
			{
				log::error << L"Failed, unable to open target stream" << Endl;
				continue;
			}

			if (!StreamCopy(targetStream, sourceStream).execute())
				log::error << L"Failed, unable to copy data" << Endl;

			targetStream->close();
			sourceStream->close();
		}

		if (!targetInstance->commit())
		{
			log::error << L"Failed, unable to commit target instance" << Endl;
			continue;
		}
	}

	for (RefArray< db::Group >::iterator i = sourceGroup->getBeginChildGroup(); i != sourceGroup->getEndChildGroup(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT (sourceChildGroup);

		log::info << L"Creating group \"" << sourceChildGroup->getName() << L"\"..." << Endl;
		status.notify(0, sourceChildGroup->getName());

		Ref< db::Group > targetChildGroup = targetGroup->getGroup(sourceChildGroup->getName());
		if (!targetChildGroup)
		{
			targetChildGroup = targetGroup->createGroup(sourceChildGroup->getName());
			if (!targetChildGroup)
			{
				log::error << L"Failed, unable to create target group" << Endl;
				continue;
			}
		}

		recursiveConvertInstances(targetChildGroup, sourceChildGroup, status);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ConvertDatabaseTool", ConvertDatabaseTool, IEditorTool)

std::wstring ConvertDatabaseTool::getDescription() const
{
	return i18n::Text(L"CONVERT_DATABASE_DESCRIPTION");
}

bool ConvertDatabaseTool::launch(ui::Widget* parent, IEditor* editor)
{
	Ref< editor::IProject > project = editor->getProject();
	Ref< db::Database > sourceDb = project->getOutputDatabase();
	if (!sourceDb)
		return false;

	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, i18n::Text(L"CONVERT_DATABASE_SAVE_AS"), L"Compact database;*.compact;All files;*.*", true))
		return false;

	Path targetDatabasePath;
	if (fileDialog.showModal(targetDatabasePath) != ui::DrOk)
	{
		fileDialog.destroy();
		return false;
	}
	fileDialog.destroy();

	Ref< db::IProviderDatabase > targetProvider;

	if (targetDatabasePath.getExtension() == L"compact")
	{
		Ref< db::CompactDatabase > targetCompactProvider = new db::CompactDatabase();
		if (!targetCompactProvider->create(targetDatabasePath))
			return false;
		targetProvider = targetCompactProvider;
	}

	Ref< db::Database > targetDb = new db::Database();
	if (!targetDb->create(targetProvider))
		return false;

	ui::custom::BackgroundWorkerStatus status(0);

	Thread* threadConvert = ThreadManager::getInstance().create(
		makeFunctor<
			ConvertDatabaseTool,
			db::Database*,
			db::Database*,
			ui::custom::BackgroundWorkerStatus&
		>(
			this,
			&ConvertDatabaseTool::threadConvert,
			sourceDb.ptr(),
			targetDb.ptr(),
			status
		),
		L"Convert database thread"
	);
	T_ASSERT (threadConvert);

	ui::custom::BackgroundWorkerDialog dialogLoader;
	if (dialogLoader.create(parent, i18n::Text(L"CONVERT_DATABASE_CONVERTING_TITLE"), i18n::Text(L"CONVERT_DATABASE_CONVERTING_MESSAGE")))
	{
		threadConvert->start();
		if (dialogLoader.execute(threadConvert, &status) != ui::DrOk)
			threadConvert->stop();
		dialogLoader.destroy();
	}

	ThreadManager::getInstance().destroy(threadConvert);

	targetDb->close();
	return true;
}

void ConvertDatabaseTool::threadConvert(db::Database* sourceDb, db::Database* targetDb, ui::custom::BackgroundWorkerStatus& status)
{
	Ref< db::Group > sourceGroup = sourceDb->getRootGroup();
	Ref< db::Group > targetGroup = targetDb->getRootGroup();
	if (sourceGroup && targetGroup)
		recursiveConvertInstances(targetGroup, sourceGroup, status);
}

	}
}
