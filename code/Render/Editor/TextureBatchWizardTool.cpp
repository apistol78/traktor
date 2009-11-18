#include "Render/Editor/TextureBatchWizardTool.h"
#include "Render/Editor/TextureBatchDialog.h"
#include "Render/Editor/TextureAsset.h"
#include "Editor/IEditor.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Ui/FileDialog.h"
#include "I18N/Text.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureBatchWizardTool", 0, TextureBatchWizardTool, editor::IWizardTool)

std::wstring TextureBatchWizardTool::getDescription() const
{
	return i18n::Text(L"TEXTURE_BATCH_WIZARDTOOL_DESCRIPTION");
}

bool TextureBatchWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group)
{
	TextureBatchDialog textureDialog(editor);

	if (!textureDialog.create(parent))
		return false;

	RefArray< TextureAsset > textureAssets;
	if (textureDialog.showModal(textureAssets))
	{
		for (RefArray< TextureAsset >::iterator i = textureAssets.begin(); i != textureAssets.end(); ++i)
		{
			std::wstring instanceName = Path((*i)->getFileName()).getFileNameNoExtension();
			Ref< db::Instance > textureAssetInstance = group->createInstance(instanceName);
			if (textureAssetInstance)
			{
				textureAssetInstance->setObject(*i);
				textureAssetInstance->commit();
			}
			else
				log::error << L"Unable to create instance \"" << instanceName << L"\"; asset not imported" << Endl;
		}
	}
	
	textureDialog.destroy();
	return true;
}

	}
}
