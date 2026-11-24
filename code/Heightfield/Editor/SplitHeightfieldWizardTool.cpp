/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/SplitHeightfieldWizardTool.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"
#include "I18N/Text.h"
#include "Ui/InputDialog.h"
#include "Ui/NumericEditValidator.h"

#include <cmath>

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.SplitHeightfieldWizardTool", 0, SplitHeightfieldWizardTool, editor::IWizardTool)

bool SplitHeightfieldWizardTool::create(const PropertyGroup* settings)
{
	return true;
}

std::wstring SplitHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"SPLIT_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet SplitHeightfieldWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< HeightfieldAsset >();
}

uint32_t SplitHeightfieldWizardTool::getFlags() const
{
	return WfInstance;
}

bool SplitHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< HeightfieldAsset > heightfieldAsset = instance->getObject< HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Unable to get heightfield asset from instance." << Endl;
		return false;
	}

	ui::InputDialog::Field fields[] = {
		ui::InputDialog::Field(L"Tile size (m)", L"512", new ui::NumericEditValidator(true, 1.0f))
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		parent,
		i18n::Text(L"SPLIT_HEIGHTFIELD_WIZARDTOOL_TITLE"),
		i18n::Text(L"SPLIT_HEIGHTFIELD_WIZARDTOOL_MESSAGE"),
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
		log::error << L"Unable to split heightfield; extent " << worldExtent.x() << L" * " << worldExtent.z() << L" m is not a multiple of tile size " << tileSize << L" m." << Endl;
		return false;
	}

	const TileStorage sourceStorage(instance, heightfieldAsset);
	const TileStorage targetStorage(instance, heightfieldAsset, tileCount);

	const int32_t size = sourceStorage.readSize();
	if (size <= 0 || (size % tileCount) != 0)
	{
		log::error << L"Unable to split heightfield; size " << size << L" not divisible into " << tileCount << L" * " << tileCount << L" tiles." << Endl;
		return false;
	}

	if (!instance->checkout())
	{
		log::error << L"Unable to checkout heightfield instance." << Endl;
		return false;
	}

	// Data is removed when committed, thus each layer is read before it's written.
	bool result = instance->removeAllData();
	for (int32_t layer = 0; layer < (int32_t)heightfieldAsset->getLayers().size() && result; ++layer)
	{
		AlignedVector< uint8_t > mask;
		Ref< Heightfield > heightfield = sourceStorage.readLayerHeightfield(layer, worldExtent, &mask);
		result = heightfield && targetStorage.writeLayerHeightfield(layer, heightfield, &mask);
	}

	heightfieldAsset->setTileCount(tileCount);
	result = result && instance->setObject(heightfieldAsset);

	if (!result || !instance->commit())
	{
		log::error << L"Unable to split heightfield; failed to write tiles." << Endl;
		instance->revert();
		return false;
	}

	log::info << L"Heightfield split into " << tileCount << L" * " << tileCount << L" tiles of " << size / tileCount << L" samples." << Endl;
	return true;
}

}
