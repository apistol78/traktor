#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Spark/Editor/MovieAsset.h"
#include "Spark/Editor/BatchDialog.h"
#include "Spark/Editor/BatchWizardTool.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.BatchWizardTool", 0, BatchWizardTool, editor::IWizardTool)

std::wstring BatchWizardTool::getDescription() const
{
	return i18n::Text(L"FLASH_BATCH_WIZARDTOOL_DESCRIPTION");
}

uint32_t BatchWizardTool::getFlags() const
{
	return WfGroup;
}

bool BatchWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	BatchDialog movieDialog(editor);

	if (!movieDialog.create(parent))
		return false;

	RefArray< MovieAsset > movieAssets;
	if (movieDialog.showModal(movieAssets))
	{
		for (auto movieAsset : movieAssets)
		{
			std::wstring instanceName = Path(movieAsset->getFileName()).getFileNameNoExtension();
			Ref< db::Instance > movieAssetInstance = group->createInstance(instanceName);
			if (movieAssetInstance)
			{
				movieAssetInstance->setObject(movieAsset);
				movieAssetInstance->commit();
			}
			else
				log::error << L"Unable to create instance \"" << instanceName << L"\"; asset not imported." << Endl;
		}
	}

	movieDialog.destroy();
	return true;
}

	}
}
