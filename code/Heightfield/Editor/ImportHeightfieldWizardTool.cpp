/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/ImportHeightfieldWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor::hf
{
	namespace
	{

Ref< drawing::Image > readRawTerrain(IStream* stream)
{
	const uint32_t fileSize = (uint32_t)stream->available();
	const uint32_t heightByteSize = 2;
	const uint32_t heights = fileSize / heightByteSize;
	const uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR16(),
		size,
		size
	);
	stream->read(image->getData(), fileSize);
	stream->close();

	return image;
}

std::wstring getUniqueInstanceName(const std::wstring& baseName, db::Group* group)
{
	if (!group->getInstance(baseName))
		return baseName;

	for (int32_t i = 2;; ++i)
	{
		const std::wstring sequenceName = baseName + L" (" + toString(i) + L")";
		if (!group->getInstance(sequenceName))
			return sequenceName;
	}

	return L"";
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ImportHeightfieldWizardTool", 0, ImportHeightfieldWizardTool, editor::IWizardTool)

std::wstring ImportHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"IMPORT_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet ImportHeightfieldWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< HeightfieldAsset >();
}

uint32_t ImportHeightfieldWizardTool::getFlags() const
{
	return WfInstance | WfGroup;
}

bool ImportHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	// Select source heightfield.
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, type_name(this), i18n::Text(L"IMPORT_HEIGHTFIELD_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", assetPath))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return false;
	}
	fileDialog.destroy();

	// Read source heightfield as image.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Failed to import heightfield; unable to open file \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > image;

	// First try reading heightfield as an image.
	image = drawing::Image::load(file, fileName.getExtension());
	//if (image)
	//	image->convert(drawing::PixelFormat::getR16());

	// Last we try as a raw image.
	if (!image)
		image = readRawTerrain(file);

	if (!image)
	{
		log::error << L"Failed to import heightfiled; unable to parse file \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	// Flip base image.
	bool invertX = false;
	bool invertZ = false;
	if (invertX || invertZ)
	{
		drawing::MirrorFilter mirrorFilter(invertX, invertZ);
		image->apply(&mirrorFilter);
	}

	uint32_t size = image->getWidth();

	// Rescale base layer to fit 2^x.
	int32_t detailSkip = 0;
	if (detailSkip > 1)
	{
		if (!(size /= detailSkip))
		{
			log::error << L"Failed to import heightfield; incorrect detail skip." << Endl;
			return false;
		}

		drawing::ScaleFilter scaleFilter(
			size,
			size,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		image->apply(&scaleFilter);
	}

	Ref< Heightfield > heightfield;
	if (instance)
	{
		// Read existing heightfield.
		Ref< HeightfieldAsset > heightfieldAsset = instance->getObject< HeightfieldAsset >();
		if (!heightfieldAsset)
		{
			log::error << L"Failed to read existing heightfield asset; incorrect instance type." << Endl;
			return false;
		}

		heightfield = new Heightfield(
			size,
			heightfieldAsset->getWorldExtent()
		);
	}
	else
	{
		// Create heightfield.
		heightfield = new Heightfield(
			size,
			Vector4(1024.0f, 128.0f, 1024.0f)
		);
	}

	const height_t* sourceHeights = static_cast< const height_t* >(image->getData());
	height_t* destinationHeights = heightfield->getHeights();
	std::memcpy(
		destinationHeights,
		sourceHeights,
		size * size * sizeof(height_t)
	);

	uint8_t* destinationCuts = heightfield->getCuts();
	std::memset(
		destinationCuts,
		0xff,
		size * size / 8
	);

	if (!instance)
	{
		// Determine unique instance name.
		std::wstring instanceName = getUniqueInstanceName(fileName.getFileNameNoExtension(), group);
		if (instanceName.empty())
			return false;

		// Create heightfield asset.
		Ref< HeightfieldAsset > heightfieldAsset = new HeightfieldAsset(heightfield->getWorldExtent());

		// Create heightfield instance.
		Ref< db::Instance > heightfieldInstance = group->createInstance(instanceName);
		heightfieldInstance->setObject(heightfieldAsset);

		Ref< IStream > data = heightfieldInstance->writeData(L"Data");
		if (!data)
		{
			log::error << L"Failed to write heightfield data into instance; import heights failed." << Endl;
			heightfieldInstance->revert();
			return false;
		}

		HeightfieldFormat().write(data, heightfield);

		heightfieldInstance->commit();
	}
	else
	{
		if (!instance->checkout())
		{
			log::error << L"Failed to write heightfield data into instance; unable to checkout instance." << Endl;
			return false;
		}

		// Replace heightfield in existing asset.
		Ref< IStream > data = instance->writeData(L"Data");
		if (!data)
		{
			log::error << L"Failed to write heightfield data into instance; import heights failed." << Endl;
			instance->revert();
			return false;
		}

		HeightfieldFormat().write(data, heightfield);

		instance->commit();
	}

	log::info << L"Heightfield imported successfully!" << Endl;
	return true;
}

}
