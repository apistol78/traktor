/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/CropHeightfieldWizardTool.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetLayer.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"
#include "I18N/Text.h"
#include "Ui/InputDialog.h"
#include "Ui/NumericEditValidator.h"

#include <cstring>

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.CropHeightfieldWizardTool", 0, CropHeightfieldWizardTool, editor::IWizardTool)

bool CropHeightfieldWizardTool::create(const PropertyGroup* settings)
{
	return true;
}

std::wstring CropHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet CropHeightfieldWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< HeightfieldAsset >();
}

uint32_t CropHeightfieldWizardTool::getFlags() const
{
	return WfInstance;
}

bool CropHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< const HeightfieldAsset > heightfieldAsset = instance->getObject< HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Unable to get heightfield asset from instance." << Endl;
		return false;
	}

	const TileStorage tileStorage(instance, heightfieldAsset);

	const int32_t originalSize = tileStorage.readSize();
	if (originalSize <= 0)
		return false;

	ui::InputDialog::Field fields[] = {
		ui::InputDialog::Field(L"Left", L"0", new ui::NumericEditValidator(false)),
		ui::InputDialog::Field(L"Top", L"0", new ui::NumericEditValidator(false)),
		ui::InputDialog::Field(L"Size", toString(originalSize), new ui::NumericEditValidator(false, 1))
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		parent,
		i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_TITLE"),
		i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_MESSAGE"),
		fields,
		sizeof_array(fields));

	if (inputDialog.showModal() == ui::DialogResult::Cancel)
	{
		inputDialog.destroy();
		return false;
	}

	const int32_t x = parseString< int32_t >(fields[0].value);
	const int32_t y = parseString< int32_t >(fields[1].value);
	const int32_t size = parseString< int32_t >(fields[2].value);

	inputDialog.destroy();

	if (size <= 0)
	{
		log::error << L"Invalid size; must be greater or equal to one." << Endl;
		return false;
	}

	if (!instance->checkout())
	{
		log::error << L"Unable to checkout heightfield instance." << Endl;
		return false;
	}

	// Each layer, with its mask, is cropped; layers without stored data are skipped.
	bool result = true;
	for (int32_t layer = 0; layer < (int32_t)heightfieldAsset->getLayers().size() && result; ++layer)
	{
		if (!heightfieldAsset->getLayers()[layer]->haveData() && !heightfieldAsset->getLayers()[layer]->haveMask())
			continue;

		AlignedVector< uint16_t > mask;
		Ref< Heightfield > heightfield = tileStorage.readLayerHeightfield(layer, heightfieldAsset->getWorldExtent(), &mask);
		if (!heightfield)
		{
			result = false;
			break;
		}

		Ref< Heightfield > cropped = new Heightfield(size, heightfield->getWorldExtent(), heightfield->getWorldCenter());
		AlignedVector< uint16_t > croppedMask((size_t)size * size);

		const uint8_t* splats = heightfield->getSplats();
		const uint8_t* colors = heightfield->getColors();
		uint8_t* croppedSplats = splats ? cropped->createSplats() : nullptr;
		uint8_t* croppedColors = colors ? cropped->createColors() : nullptr;

		for (int32_t iy = 0; iy < size; ++iy)
		{
			for (int32_t ix = 0; ix < size; ++ix)
			{
				const int32_t sx = clamp(ix + x, 0, originalSize - 1);
				const int32_t sy = clamp(iy + y, 0, originalSize - 1);

				cropped->setGridHeight(ix, iy, heightfield->getGridHeightNearest(sx, sy));
				cropped->setGridCut(ix, iy, heightfield->getGridCut(sx, sy));

				for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
					if (heightfield->getAttributes(a))
						cropped->setGridAttribute(ix, iy, a, heightfield->getGridAttributeDensity(sx, sy, a));

				const size_t dst = (size_t)ix + (size_t)iy * size;
				const size_t src = (size_t)sx + (size_t)sy * originalSize;
				if (croppedSplats)
					std::memcpy(croppedSplats + dst * 4, splats + src * 4, 4);
				if (croppedColors)
					std::memcpy(croppedColors + dst * 4, colors + src * 4, 4);
				croppedMask[dst] = mask[src];
			}
		}

		result = tileStorage.writeLayerHeightfield(layer, cropped, &croppedMask);
	}

	if (!result || !instance->commit())
	{
		log::error << L"Unable to crop heightfield; failed to write heightfield." << Endl;
		instance->revert();
		return false;
	}

	return true;
}

}
