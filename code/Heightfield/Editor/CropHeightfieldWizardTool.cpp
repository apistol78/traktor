#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/CropHeightfieldWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.CropHeightfieldWizardTool", 0, CropHeightfieldWizardTool, editor::IWizardTool)

std::wstring CropHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

uint32_t CropHeightfieldWizardTool::getFlags() const
{
	return WfInstance;
}

bool CropHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	if (!instance->checkout())
	{
		log::error << L"Unable to checkout heightfield asset" << Endl;
		return false;
	}

	Ref< HeightfieldAsset > heightfieldAsset = instance->getObject< HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Unable to get heightfield asset from instance" << Endl;
		return false;
	}

	Ref< IStream > sourceData = instance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Unable to open heightfield data stream" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		log::error << L"Unable to read heightfield from data stream" << Endl;
		return false;
	}

	sourceData->close();
	sourceData = 0;

	ui::custom::InputDialog::Field fields[] =
	{
		{ L"Left", L"0", new ui::NumericEditValidator(false), 0 },
		{ L"Top", L"0", new ui::NumericEditValidator(false), 0 },
		{ L"Size", toString(heightfield->getSize()), new ui::NumericEditValidator(false, 1), 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		parent,
		i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_TITLE"),
		i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_MESSAGE"),
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

	float factor = float(size) / heightfield->getSize();

	Ref< Heightfield > cropped = new Heightfield(
		size,
		heightfield->getWorldExtent() * Vector4(factor, 1.0f, factor)
	);

	for (int32_t iy = 0; iy < size; ++iy)
	{
		for (int32_t ix = 0; ix < size; ++ix)
		{
			int32_t sx = ix + x;
			int32_t sy = iy + y;

			sx = clamp(sx, 0, heightfield->getSize() - 1);
			sy = clamp(sy, 0, heightfield->getSize() - 1);

			float h = heightfield->getGridHeightNearest(sx, sy);
			bool c = heightfield->getGridCut(sx, sy);

			cropped->setGridHeight(ix, iy, h);
			cropped->setGridCut(ix, iy, c);
		}
	}

	sourceData = instance->writeData(L"Data");
	if (!sourceData)
		return false;

	HeightfieldFormat().write(
		sourceData,
		cropped
	);
	if (!heightfield)
		return false;

	sourceData->close();
	sourceData = 0;

	instance->setObject(new HeightfieldAsset(cropped->getWorldExtent()));

	if (!instance->commit())
		return false;

	return true;
}

	}
}
