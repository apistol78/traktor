#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ExportHeightfieldWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

Ref< Heightfield > readOld(IStream* stream, const Vector4& worldExtent)
{
	uint32_t fileSize = stream->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< Heightfield > hf = new Heightfield(size, worldExtent);

	stream->read(hf->getHeights(), size * size * sizeof(height_t));

	stream->close();

	uint8_t* destinationCuts = hf->getCuts();
	std::memset(
		destinationCuts,
		0xff,
		size * size / 8
	);

	return hf;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ExportHeightfieldWizardTool", 0, ExportHeightfieldWizardTool, editor::IWizardTool)

std::wstring ExportHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"EXPORT_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

uint32_t ExportHeightfieldWizardTool::getFlags() const
{
	return WfInstance;
}

bool ExportHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< HeightfieldAsset > heightfieldAsset = instance->getObject< HeightfieldAsset >();
	if (!heightfieldAsset)
		return false;

	Ref< IStream > sourceData = instance->readData(L"Data");
	if (!sourceData)
		return false;

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		heightfield = readOld(sourceData, heightfieldAsset->getWorldExtent());
		if (!heightfield)
			return false;
	}

	sourceData->close();
	sourceData = 0;

	log::info << L"Heightfield size " << heightfield->getSize() << Endl;

	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, i18n::Text(L"EXPORT_HEIGHTFIELD_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", true);
	if (saveAsDialog.showModal(fileName) != ui::DrOk)
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
		return false;

	//HeightfieldFormat().write(file, heightfield);

	file->write(
		heightfield->getHeights(),
		heightfield->getSize() * heightfield->getSize() * sizeof(height_t)
	);

	file->close();
	file = 0;

	return true;
}

	}
}
