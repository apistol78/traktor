#include <limits>
#include "Core/Log/Log.h"
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/ExportHeightfieldAsMeshWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Model/ModelFormat.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ExportHeightfieldAsMeshWizardTool", 0, ExportHeightfieldAsMeshWizardTool, editor::IWizardTool)

std::wstring ExportHeightfieldAsMeshWizardTool::getDescription() const
{
	return i18n::Text(L"EXPORT_HEIGHTFIELD_AS_MESH_WIZARDTOOL_DESCRIPTION");
}

uint32_t ExportHeightfieldAsMeshWizardTool::getFlags() const
{
	return WfInstance;
}

bool ExportHeightfieldAsMeshWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
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
	sourceData = nullptr;

	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, type_name(this), i18n::Text(L"EXPORT_HEIGHTFIELD_AS_MESH_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", true);
	if (saveAsDialog.showModal(fileName) != ui::DrOk)
		return false;

	Ref< model::Model > model = ConvertHeightfield().convert(heightfield, 16, std::numeric_limits< float >::max());
	if (!model)
		return false;

	return model::ModelFormat::writeAny(fileName, model);
}

	}
}
