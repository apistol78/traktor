#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/Asset.h"
#include "Editor/IEditor.h"
#include "Editor/App/CheckAssetsTool.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.CheckAssetsTool", 0, CheckAssetsTool, IEditorTool)

std::wstring CheckAssetsTool::getDescription() const
{
	return i18n::Text(L"EDITOR_CHECK_ASSETS");
}

Ref< ui::IBitmap > CheckAssetsTool::getIcon() const
{
	return 0;
}

bool CheckAssetsTool::launch(ui::Widget* parent, IEditor* editor)
{
	Ref< db::Database > database = editor->getSourceDatabase();
	if (!database)
		return true;

	std::wstring assetPath = editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	RefArray< db::Instance > assetInstances;
	db::recursiveFindChildInstances(
		database->getRootGroup(),
		db::FindInstanceByType(type_of< Asset >()),
		assetInstances
	);

	int32_t errorCount = 0;

	for (RefArray< db::Instance >::const_iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
	{
		Ref< Asset > asset = (*i)->getObject< Asset >();
		if (!asset)
		{
			log::error << L"Unable to read asset \"" << (*i)->getPath() << L"\"" << Endl;
			++errorCount;
			continue;
		}

		if (!asset->getFileName().isRelative())
		{
			log::error << L"File \"" << asset->getFileName().getPathName() << L"\" absolute, in asset \"" << (*i)->getPath() << L"\"" << Endl;
			++errorCount;
		}

		Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

		RefArray< File > files;
		FileSystem::getInstance().find(fileName, files);

		if (files.empty())
		{
			log::error << L"File \"" << fileName.getPathName() << L"\" missing, in asset \"" << (*i)->getPath() << L"\"" << Endl;
			++errorCount;
		}
	}

	log::info << L"Check asset completed; " << errorCount << L" error(s) found in " << int32_t(assetInstances.size()) << L" asset(s)" << Endl;
	return true;
}

	}
}
