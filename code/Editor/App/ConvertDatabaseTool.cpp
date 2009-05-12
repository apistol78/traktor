#include "Editor/App/ConvertDatabaseTool.h"
#include "Editor/Editor.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Ui/FileDialog.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "I18N/Text.h"
#include "Core/Io/Stream.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

void recursiveConvertInstances(db::Group* targetGroup, db::Group* sourceGroup, ui::custom::BackgroundWorkerStatus& status)
{
	for (Ref< db::Instance > sourceInstance = sourceGroup->getFirstChildInstance(); sourceInstance; sourceInstance = sourceGroup->getNextChildInstance(sourceInstance))
	{
		log::info << L"Converting \"" << sourceInstance->getName() << L"\"..." << Endl;
		status.notify(0, sourceInstance->getName());

		Ref< Serializable > sourceObject = sourceInstance->checkout(db::CfReadOnly);
		if (!sourceObject)
		{
			log::error << L"Failed, unable to checkout source instance" << Endl;
			continue;
		}

		Guid sourceGuid = sourceInstance->getGuid();
		Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), sourceObject, db::CifReplaceExisting, &sourceGuid);
		if (!targetInstance)
		{
			log::error << L"Failed, unable to create target instance" << Endl;
			continue;
		}

		std::vector< std::wstring > dataNames;
		sourceInstance->getDataNames(dataNames);

		for (std::vector< std::wstring >::iterator i = dataNames.begin(); i != dataNames.end(); ++i)
		{
			log::info << L"Copying data \"" << *i << L"\"..." << Endl;

			Ref< Stream > sourceStream = sourceInstance->readData(*i);
			if (!sourceStream)
			{
				log::error << L"Failed, unable to open source stream" << Endl;
				continue;
			}

			Ref< Stream > targetStream = targetInstance->writeData(*i);
			if (!targetStream)
			{
				log::error << L"Failed, unable to open target stream" << Endl;
				continue;
			}

			uint8_t block[1024]; int blockSize;
			while ((blockSize = sourceStream->read(block, sizeof(block))) > 0)
				targetStream->write(block, blockSize);

			targetStream->close();
			sourceStream->close();
		}

		if (!targetInstance->commit())
		{
			log::error << L"Failed, unable to commit target instance" << Endl;
			continue;
		}

		log::info << L"Succeeded" << Endl;
	}

	for (Ref< db::Group > sourceChildGroup = sourceGroup->getFirstChildGroup(); sourceChildGroup; sourceChildGroup = sourceGroup->getNextChildGroup(sourceChildGroup))
	{
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

		log::info << L"Succeeded" << Endl;

		recursiveConvertInstances(targetChildGroup, sourceChildGroup, status);
	}
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.ConvertDatabaseTool", ConvertDatabaseTool, EditorTool)

std::wstring ConvertDatabaseTool::getDescription() const
{
	return i18n::Text(L"CONVERT_DATABASE_DESCRIPTION");
}

bool ConvertDatabaseTool::launch(ui::Widget* parent, Editor* editor)
{
	Ref< db::Database > sourceDb = editor->getOutputDatabase();
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
		Ref< db::CompactDatabase > targetCompactProvider = gc_new< db::CompactDatabase >();
		if (!targetCompactProvider->create(targetDatabasePath))
			return false;
		targetProvider = targetCompactProvider;
	}

	Ref< db::Database > targetDb = gc_new< db::Database >();
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
			sourceDb.getPtr(),
			targetDb.getPtr(),
			status
		),
		L"Convert database thread"
	);
	T_ASSERT (threadConvert);

	ui::custom::BackgroundWorkerDialog dialogLoader;
	if (dialogLoader.create(parent, i18n::Text(L"CONVERT_DATABASE_CONVERTING_TITLE"), i18n::Text(L"CONVERT_DATABASE_CONVERTING_MESSAGE")))
	{
		dialogLoader.execute(threadConvert, &status);
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
