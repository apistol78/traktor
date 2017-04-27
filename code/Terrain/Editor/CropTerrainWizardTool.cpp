/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "I18N/Text.h"
#include "Terrain/Editor/CropTerrainWizardTool.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.CropTerrainWizardTool", 0, CropTerrainWizardTool, editor::IWizardTool)

std::wstring CropTerrainWizardTool::getDescription() const
{
	return i18n::Text(L"CROP_TERRAIN_WIZARDTOOL_DESCRIPTION");
}

uint32_t CropTerrainWizardTool::getFlags() const
{
	return WfInstance;
}

bool CropTerrainWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< IStream > file;
	Ref< drawing::Image > colorImage;
	Ref< drawing::Image > splatImage;

	if (!instance->checkout())
	{
		log::error << L"Unable to checkout terrain asset" << Endl;
		return false;
	}

	Ref< TerrainAsset > terrainAsset = instance->getObject< TerrainAsset >();
	if (!terrainAsset)
	{
		log::error << L"Unable to get terrain asset from instance" << Endl;
		return false;
	}

	file = instance->readData(L"Color");
	if (file)
	{
		colorImage = drawing::Image::load(file, L"tga");
		file->close();
		file = 0;
	}

	file = instance->readData(L"Splat");
	if (file)
	{
		splatImage = drawing::Image::load(file, L"tga");
		file->close();
		file = 0;
	}

	if (!colorImage && !splatImage)
	{
		log::error << L"Terrain doesn't have neither a color nor a splat map, thus no need to crop" << Endl;
		return true;
	}

	int32_t originalSize = 0;
	if (colorImage)
	{
		T_ASSERT (colorImage->getWidth() == colorImage->getHeight());
		originalSize = colorImage->getWidth();
	}
	else if (splatImage)
	{
		T_ASSERT (splatImage->getWidth() == splatImage->getHeight());
		originalSize = splatImage->getWidth();
	}
	T_ASSERT (originalSize > 0);

	ui::custom::InputDialog::Field fields[] =
	{
		{ L"Left", L"0", new ui::NumericEditValidator(false), 0 },
		{ L"Top", L"0", new ui::NumericEditValidator(false), 0 },
		{ L"Size", toString(originalSize), new ui::NumericEditValidator(false, 1), 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		parent,
		i18n::Text(L"CROP_TERRAIN_WIZARDTOOL_TITLE"),
		i18n::Text(L"CROP_TERRAIN_WIZARDTOOL_MESSAGE"),
		fields,
		sizeof_array(fields)
	);

	if (inputDialog.showModal() == ui::DrCancel)
		return false;

	int32_t x = parseString< int32_t >(fields[0].value);
	int32_t y = parseString< int32_t >(fields[1].value);
	int32_t size = parseString< int32_t >(fields[2].value);

	if (size <= 0)
	{
		log::error << L"Invalid size; must be greater or equal to one" << Endl;
		return false;
	}

	Ref< drawing::Image > croppedColorImage;
	Ref< drawing::Image > croppedSplatImage;

	if (colorImage)
		croppedColorImage = new drawing::Image(colorImage->getPixelFormat(), size, size);
	if (splatImage)
		croppedSplatImage = new drawing::Image(splatImage->getPixelFormat(), size, size);

	for (int32_t iy = 0; iy < size; ++iy)
	{
		for (int32_t ix = 0; ix < size; ++ix)
		{
			int32_t sx = ix + x;
			int32_t sy = iy + y;

			sx = clamp(sx, 0, originalSize - 1);
			sy = clamp(sy, 0, originalSize - 1);

			if (colorImage)
			{
				Color4f color;
				colorImage->getPixel(sx, sy, color);
				croppedColorImage->setPixel(ix, iy, color);
			}

			if (splatImage)
			{
				Color4f color;
				splatImage->getPixel(sx, sy, color);
				croppedSplatImage->setPixel(ix, iy, color);
			}
		}
	}

	if (croppedColorImage)
	{
		file = instance->writeData(L"Color");
		if (file)
		{
			croppedColorImage->save(file, L"tga");
			file->close();
			file = 0;
		}
	}

	if (croppedSplatImage)
	{
		file = instance->writeData(L"Splat");
		if (file)
		{
			croppedSplatImage->save(file, L"tga");
			file->close();
			file = 0;
		}
	}

	if (!instance->commit())
		return false;

	return true;
}

	}
}
