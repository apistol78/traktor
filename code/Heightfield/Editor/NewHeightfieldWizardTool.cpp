#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/NewHeightfieldWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.NewHeightfieldWizardTool", 0, NewHeightfieldWizardTool, editor::IWizardTool)

std::wstring NewHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"NEW_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

uint32_t NewHeightfieldWizardTool::getFlags() const
{
	return WfGroup;
}

bool NewHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	const Vector4 worldExtent(1024.0f, 512.0f, 1024.0f, 0.0f);
	const int32_t gridSize = 2048;

	Ref< db::Instance > heightfieldInstance = group->createInstance(L"Unnamed");
	if (!heightfieldInstance)
		return false;

	Ref< Heightfield > heightfield = new Heightfield(gridSize, worldExtent);
	
	for (int32_t z = 0; z < gridSize; ++z)
	{
		for (int32_t x = 0; x < gridSize; ++x)
		{
			heightfield->setGridHeight(x, z, 0.5f);
			heightfield->setGridCut(x, z, true);
		}
	}

	Ref< IStream > file = heightfieldInstance->writeData(L"Data");
	if (!file)
		return false;

	HeightfieldFormat().write(file, heightfield);

	file->close();
	file = 0;

	Ref< HeightfieldAsset > heightfieldAsset = new HeightfieldAsset(worldExtent);
	heightfieldInstance->setObject(heightfieldAsset);
	heightfieldInstance->commit();

	return true;
}

	}
}
