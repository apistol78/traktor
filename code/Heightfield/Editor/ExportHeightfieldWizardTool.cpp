#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/ExportHeightfieldWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldFormat.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace hf
	{

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
		return false;

	sourceData->close();
	sourceData = 0;

	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, L"Save heightfield as...", L"All files;*.*", true);
	if (saveAsDialog.showModal(fileName) != ui::DrOk)
		return false;


	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
		return false;

	HeightfieldFormat().write(file, heightfield);

	file->close();
	file = 0;

	return true;
}

	}
}
