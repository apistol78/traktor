/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/SplitTerrainWizardTool.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IEditor.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"
#include "I18N/Text.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Ui/InputDialog.h"
#include "Ui/NumericEditValidator.h"

#include <cmath>

namespace traktor::terrain
{
namespace
{

bool divisible(const drawing::Image* image, int32_t tileCount)
{
	return image == nullptr || ((image->getWidth() % tileCount) == 0 && (image->getHeight() % tileCount) == 0);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.SplitTerrainWizardTool", 0, SplitTerrainWizardTool, editor::IWizardTool)

bool SplitTerrainWizardTool::create(const PropertyGroup* settings)
{
	return true;
}

std::wstring SplitTerrainWizardTool::getDescription() const
{
	return i18n::Text(L"SPLIT_TERRAIN_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet SplitTerrainWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< TerrainAsset >();
}

uint32_t SplitTerrainWizardTool::getFlags() const
{
	return WfInstance;
}

bool SplitTerrainWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< const TerrainAsset > terrainAsset = instance->getObject< TerrainAsset >();
	if (!terrainAsset)
	{
		log::error << L"Unable to get terrain asset from instance." << Endl;
		return false;
	}

	Ref< db::Instance > heightfieldInstance = editor->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
	if (!heightfieldInstance)
	{
		log::error << L"Unable to get heightfield instance of terrain." << Endl;
		return false;
	}

	Ref< hf::HeightfieldAsset > heightfieldAsset = heightfieldInstance->getObject< hf::HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Unable to get heightfield asset of terrain." << Endl;
		return false;
	}

	ui::InputDialog::Field fields[] = {
		ui::InputDialog::Field(L"Tile size (m)", L"512", new ui::NumericEditValidator(true, 1.0f))
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		parent,
		i18n::Text(L"SPLIT_TERRAIN_WIZARDTOOL_TITLE"),
		i18n::Text(L"SPLIT_TERRAIN_WIZARDTOOL_MESSAGE"),
		fields,
		sizeof_array(fields));

	if (inputDialog.showModal() == ui::DialogResult::Cancel)
	{
		inputDialog.destroy();
		return false;
	}

	const float tileSize = parseString< float >(fields[0].value);

	inputDialog.destroy();

	const Vector4& worldExtent = heightfieldAsset->getWorldExtent();
	const int32_t tileCount = tileSize > 0.0f ? (int32_t)std::floor(worldExtent.x() / tileSize + 0.5f) : 0;
	if (
		tileCount <= 0 ||
		std::abs(tileCount * tileSize - worldExtent.x()) > 0.01f ||
		std::abs(tileCount * tileSize - worldExtent.z()) > 0.01f)
	{
		log::error << L"Unable to split terrain; extent " << worldExtent.x() << L" * " << worldExtent.z() << L" m is not a multiple of tile size " << tileSize << L" m." << Endl;
		return false;
	}

	// Read everything before anything is modified.
	const hf::TileStorage sourceStorage(instance, heightfieldAsset->getTileCount());
	const hf::TileStorage targetStorage(heightfieldInstance, tileCount);

	Ref< hf::Heightfield > heightfield = sourceStorage.readHeightfield(worldExtent);
	if (!heightfield)
		return false;

	Ref< drawing::Image > colorImage = sourceStorage.readImage(L"Color");
	Ref< drawing::Image > splatImage = sourceStorage.readImage(L"Splat");

	if ((heightfield->getSize() % tileCount) != 0 || !divisible(colorImage, tileCount) || !divisible(splatImage, tileCount))
	{
		log::error << L"Unable to split terrain; heightfield or maps not divisible into " << tileCount << L" * " << tileCount << L" tiles." << Endl;
		return false;
	}

	if (!heightfieldInstance->checkout())
	{
		log::error << L"Unable to checkout heightfield instance." << Endl;
		return false;
	}

	if (!instance->checkout())
	{
		log::error << L"Unable to checkout terrain instance." << Endl;
		heightfieldInstance->revert();
		return false;
	}

	heightfieldAsset->setTileCount(tileCount);

	const bool result =
		heightfieldInstance->removeAllData() &&
		targetStorage.writeHeightfield(heightfield) &&
		heightfieldInstance->setObject(heightfieldAsset) &&
		instance->removeAllData() &&
		(!colorImage || targetStorage.writeImage(L"Color", colorImage)) &&
		(!splatImage || targetStorage.writeImage(L"Splat", splatImage));

	if (!result || !heightfieldInstance->commit() || !instance->commit())
	{
		log::error << L"Unable to split terrain; failed to write tiles." << Endl;
		heightfieldInstance->revert();
		instance->revert();
		return false;
	}

	log::info << L"Terrain split into " << tileCount << L" * " << tileCount << L" tiles of " << heightfield->getSize() / tileCount << L" samples." << Endl;
	return true;
}

}
