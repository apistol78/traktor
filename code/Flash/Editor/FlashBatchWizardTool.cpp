/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashBatchDialog.h"
#include "Flash/Editor/FlashBatchWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBatchWizardTool", 0, FlashBatchWizardTool, editor::IWizardTool)

std::wstring FlashBatchWizardTool::getDescription() const
{
	return i18n::Text(L"FLASH_BATCH_WIZARDTOOL_DESCRIPTION");
}

uint32_t FlashBatchWizardTool::getFlags() const
{
	return WfGroup;
}

bool FlashBatchWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	FlashBatchDialog movieDialog(editor);

	if (!movieDialog.create(parent))
		return false;

	RefArray< FlashMovieAsset > movieAssets;
	if (movieDialog.showModal(movieAssets))
	{
		for (RefArray< FlashMovieAsset >::iterator i = movieAssets.begin(); i != movieAssets.end(); ++i)
		{
			std::wstring instanceName = Path((*i)->getFileName()).getFileNameNoExtension();
			Ref< db::Instance > movieAssetInstance = group->createInstance(instanceName);
			if (movieAssetInstance)
			{
				movieAssetInstance->setObject(*i);
				movieAssetInstance->commit();
			}
			else
				log::error << L"Unable to create instance \"" << instanceName << L"\"; asset not imported" << Endl;
		}
	}
	
	movieDialog.destroy();
	return true;
}

	}
}
