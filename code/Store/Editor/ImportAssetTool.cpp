#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
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

bool copyFiles(const Path& targetPath, const Path& sourcePath)
{
	if (!FileSystem::getInstance().makeAllDirectories(targetPath))
		return false;

	RefArray< File > files;
	FileSystem::getInstance().find(sourcePath.getPathName() + L"/*.*", files);
	
	for (auto file : files)
	{
		auto p = file->getPath();
		if (p.getFileName() == L"." || p.getFileName() == L"..")
			continue;

		if (!file->isDirectory())
		{
			if (!FileSystem::getInstance().copy(
				targetPath.getPathName() + L"/" + p.getFileName(),
				sourcePath.getPathName() + L"/" + p.getFileName()
			))
				return false;

		}
		else
		{
			if (!copyFiles(
				targetPath.getPathName() + L"/" + p.getFileName(),
				sourcePath.getPathName() + L"/" + p.getFileName()
			))
				return false;
		}
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
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, type_name(this), L"Select asset bundle to import", L"All files;*.*"))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return false;
	}
	fileDialog.destroy();

	// Migrate instances from bundle's database into workspace source database.
	Ref< db::Database > database = new db::Database();
	if (!database->open(L"provider=traktor.db.CompactDatabase;fileName=" + fileName.getPathName() + L";readOnly=true"))
		return false;

	log::info << L"Mergin instances from database..." << Endl;

	Ref< db::Group > sourceGroup = database->getRootGroup();
	Ref< db::Group > targetGroup = editor->getSourceDatabase()->getRootGroup();

	if (!migrateGroup(targetGroup, sourceGroup))
		return false;

	// Copy assets from bundle into workspace's assets.
	Path sourceAssetsPath = fileName.getPathOnly() + L"/Assets";
	Path destinationAssetsPath = FileSystem::getInstance().getAbsolutePath(Path(L"data/Assets"));

	if (!copyFiles(destinationAssetsPath, sourceAssetsPath))
		return false;

	log::info << L"Bundle imported successfully." << Endl;
	return true;
}

	}
}
