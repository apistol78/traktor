#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundBatchDialog.h"
#include "Sound/Editor/SoundBatchWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundBatchWizardTool", 0, SoundBatchWizardTool, editor::IWizardTool)

std::wstring SoundBatchWizardTool::getDescription() const
{
	return i18n::Text(L"SOUND_BATCH_WIZARDTOOL_DESCRIPTION");
}

uint32_t SoundBatchWizardTool::getFlags() const
{
	return WfGroup;
}

bool SoundBatchWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	SoundBatchDialog soundDialog(editor);

	if (!soundDialog.create(parent))
		return false;

	RefArray< SoundAsset > soundAssets;
	if (soundDialog.showModal(soundAssets))
	{
		for (RefArray< SoundAsset >::iterator i = soundAssets.begin(); i != soundAssets.end(); ++i)
		{
			std::wstring instanceName = Path((*i)->getFileName()).getFileNameNoExtension();
			Ref< db::Instance > soundAssetInstance = group->createInstance(instanceName);
			if (soundAssetInstance)
			{
				soundAssetInstance->setObject(*i);
				soundAssetInstance->commit();
			}
			else
				log::error << L"Unable to create instance \"" << instanceName << L"\"; asset not imported" << Endl;
		}
	}
	
	soundDialog.destroy();
	return true;
}

	}
}
