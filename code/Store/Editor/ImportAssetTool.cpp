#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Store/Editor/BrowseAssetDialog.h"
#include "Store/Editor/ImportAssetTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace store
	{
		namespace
		{

bool migrateInstance(Ref< db::Instance > sourceInstance, Ref< db::Group > targetGroup)
{
	Ref< ISerializable > sourceObject = sourceInstance->getObject();
	if (!sourceObject)
	{
		log::error << L"Failed, unable to get source object" << Endl;
		return false;
	}

	Guid sourceGuid = sourceInstance->getGuid();

	std::vector< std::wstring > dataNames;
	sourceInstance->getDataNames(dataNames);

	Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), db::CifReplaceExisting, &sourceGuid);
	if (!targetInstance)
	{
		log::error << L"Failed, unable to create target instance" << Endl;
		return false;
	}

	targetInstance->setObject(sourceObject);

	for (std::vector< std::wstring >::iterator j = dataNames.begin(); j != dataNames.end(); ++j)
	{
		Ref< IStream > sourceStream = sourceInstance->readData(*j);
		if (!sourceStream)
		{
			log::error << L"Failed, unable to open source stream" << Endl;
			return false;
		}

		Ref< IStream > targetStream = targetInstance->writeData(*j);
		if (!targetStream)
		{
			log::error << L"Failed, unable to open target stream" << Endl;
			return false;
		}

		if (!StreamCopy(targetStream, sourceStream).execute())
		{
			log::error << L"Failed, unable to copy data" << Endl;
			return false;
		}

		targetStream->close();
		sourceStream->close();
	}

	if (!targetInstance->commit())
	{
		log::error << L"Failed, unable to commit target instance" << Endl;
		return false;
	}

	return true;
}

bool migrateGroup(db::Group* targetGroup, db::Group* sourceGroup)
{
	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT(sourceInstance);

		if (!migrateInstance(sourceInstance, targetGroup))
			return false;
	}

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT(sourceChildGroup);

		Ref< db::Group > targetChildGroup = targetGroup->getGroup(sourceChildGroup->getName());
		if (!targetChildGroup)
		{
			targetChildGroup = targetGroup->createGroup(sourceChildGroup->getName());
			if (!targetChildGroup)
				return false;
		}

		if (!migrateGroup(targetChildGroup, sourceChildGroup))
			return false;
	}

	return true;
}

bool copyFiles(const Path& targetPath, db::Group* sourceGroup)
{
	if (!FileSystem::getInstance().makeAllDirectories(targetPath))
		return false;

	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT(sourceInstance);

		std::wstring targetFileName = targetPath.getPathName() + L"/" + sourceInstance->getName();

		log::info << L"Unpacking \"" << sourceInstance->getPath() << L"\" as \"" << targetFileName << L"\"..." << Endl;

		Ref< IStream > fileStream = FileSystem::getInstance().open(targetFileName, File::FmWrite);
		if (!fileStream)
		{
			log::error << L"Unable to create file \"" << targetFileName << L"\"." << Endl;
			return false;
		}

		Ref< IStream > assetStream = sourceInstance->readData(L"Data");
		if (!assetStream)
		{
			log::error << L"Unable to open source stream." << Endl;
			return false;
		}

		if (!StreamCopy(fileStream, assetStream).execute())
		{
			log::error << L"Unable to copy file." << Endl;
			return false;
		}
	}

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT(sourceChildGroup);

		if (!copyFiles(Path(targetPath.getPathName() + L"/" + sourceChildGroup->getName()), sourceChildGroup))
			return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.store.ImportAssetTool", 0, ImportAssetTool, IEditorTool)

std::wstring ImportAssetTool::getDescription() const
{
	return i18n::Text(L"STORE_IMPORT_ASSET_TOOL_DESCRIPTION");
}

Ref< ui::IBitmap > ImportAssetTool::getIcon() const
{
	return nullptr;
}

bool ImportAssetTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ImportAssetTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	RefArray< net::Url > urls;

	std::wstring serverHost = editor->getSettings()->getProperty< std::wstring >(L"Store.Server", L"127.0.0.1:8118");
	if (serverHost.empty())
		return false;

	// Let user selected packages to import.
	BrowseAssetDialog browseAssetDialog(serverHost);
	browseAssetDialog.create(parent);
	if (browseAssetDialog.showModal(urls) != ui::DrOk)
	{
		browseAssetDialog.destroy();
		return false;
	}
	browseAssetDialog.destroy();

	// Ensure temporary folder for download exist.
	if (!FileSystem::getInstance().makeAllDirectories(Path(L"$(TRAKTOR_HOME)/data/Temp/Store/Download")))
		return false;

	// Download and each selected package.
	Ref< net::HttpClient > httpClient = new net::HttpClient();
	
	RefArray< net::HttpClientResult > downloadQueries;
	for (auto url : urls)
	{	
		auto downloadQuery = httpClient->get(*url);
		if (!downloadQuery)
			return false;
		if (!downloadQuery->succeeded())
			return false;

		auto tempFile = FileSystem::getInstance().open(Path(L"$(TRAKTOR_HOME)/data/Temp/Store/Download/Database.compact"), File::FmWrite);
		if (!tempFile)
			return false;

		auto stream = downloadQuery->getStream();
		if (!StreamCopy(tempFile, stream).execute())
			return false;

		tempFile->close();
		tempFile = nullptr;

		// Migrate instances from bundle's database into workspace source database.
		Ref< db::Database > database = new db::Database();
		if (!database->open(db::ConnectionString(L"provider=traktor.db.CompactDatabase;fileName=$(TRAKTOR_HOME)/data/Temp/Store/Download/Database.compact;readOnly=true")))
			return false;

		log::info << L"Merging instances from database..." << Endl;

		Ref< db::Group > sourceGroup = database->getGroup(L"Instances");
		if (sourceGroup != nullptr)
		{
			Ref< db::Group > targetGroup = editor->getSourceDatabase()->getRootGroup();
			if (!migrateGroup(targetGroup, sourceGroup))
				return false;
		}

		// Copy embedded assets from bundle into workspace's assets.
		log::info << L"Unpacking assets from database..." << Endl;

		Ref< db::Group > assetGroup = database->getGroup(L"Assets");
		if (assetGroup != nullptr)
		{
			Path destinationAssetsPath = FileSystem::getInstance().getAbsolutePath(Path(L"data/Assets"));
			if (!copyFiles(destinationAssetsPath, assetGroup))
				return false;
		}

		database->close();
		database = nullptr;
	}

	log::info << L"Package(s) imported successfully." << Endl;
	return true;
}

	}
}
