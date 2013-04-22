#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/ImportHeightfieldWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

Ref< drawing::Image > readRawTerrain(IStream* stream)
{
	uint32_t fileSize = stream->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		drawing::PixelFormat::getR16(),
		size,
		size
	);

	stream->read(image->getData(), fileSize);
	stream->close();

	return image;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ImportHeightfieldWizardTool", 0, ImportHeightfieldWizardTool, editor::IWizardTool)

std::wstring ImportHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"IMPORT_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

uint32_t ImportHeightfieldWizardTool::getFlags() const
{
	return WfGroup;
}

bool ImportHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	// Select source heightfield.
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, i18n::Text(L"IMPORT_HEIGHTFIELD_WIZARDTOOL_FILE_TITLE"), L"All files;*.*"))
		return 0;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	// Read source heightfield as image.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return false;

	Ref< drawing::Image > image;
	if ((image = readRawTerrain(file)) == 0)
		return false;

	// Flip base image.
	bool invertX = false;
	bool invertZ = false;
	if (invertX || invertZ)
	{
		drawing::MirrorFilter mirrorFilter(invertX, invertZ);
		image = image->applyFilter(&mirrorFilter);
		T_ASSERT (image);
	}

	uint32_t size = image->getWidth();

	// Rescale base layer to fit 2^x.
	int32_t detailSkip = 0;
	if (detailSkip > 1)
	{
		if (!(size /= detailSkip))
			return 0;

		drawing::ScaleFilter scaleFilter(
			size,
			size,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		image = image->applyFilter(&scaleFilter);
		T_ASSERT (image);
	}

	// Create heightfield.
	Ref< Heightfield > heightfield = new Heightfield(
		size,
		Vector4(1024.0f, 128.0f, 1024.0f)
	);

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

	// Create heightfield asset.
	Ref< HeightfieldAsset > heightfieldAsset = new HeightfieldAsset(heightfield->getWorldExtent());

	// Create heightfield instance.
	Ref< db::Instance > heightfieldInstance = group->createInstance(fileName.getFileNameNoExtension());

	heightfieldInstance->setObject(heightfieldAsset);

	Ref< IStream > data = heightfieldInstance->writeData(L"Data");
	if (!data)
	{
		heightfieldInstance->revert();
		return false;
	}

	HeightfieldFormat().write(data, heightfield);

	heightfieldInstance->commit();

	return true;
}

	}
}
