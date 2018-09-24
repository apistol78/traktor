/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ExportHeightfieldAsMeshWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Model/Model.h"
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
	sourceData = 0;

	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, i18n::Text(L"EXPORT_HEIGHTFIELD_AS_MESH_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", true);
	if (saveAsDialog.showModal(fileName) != ui::DrOk)
		return false;

	int32_t step = 16;
	int32_t size = heightfield->getSize();
	int32_t outputSize = size / step;

	log::info << L"Heightfield size " << size << L" * " << size << L" heights" << Endl;
	log::info << L"Output mesh size " << outputSize << L" * " << outputSize << L" vertices" << Endl;

	Ref< model::Model > model = new model::Model();

	model->reservePositions(outputSize * outputSize);

	model::Vertex vertex;
	for (int32_t iz = 0; iz < size; iz += step)
	{
		for (int32_t ix = 0; ix < size; ix += step)
		{
			float wx, wz;
			heightfield->gridToWorld(ix, iz, wx, wz);

			uint32_t positionId = model->addPosition(Vector4(
				wx,
				heightfield->getWorldHeight(wx, wz),
				wz,
				1.0f
			));

			uint32_t texCoordId = model->addTexCoord(Vector2(
				float(ix) / (size - 1),
				float(iz) / (size - 1)
			));

			vertex.setPosition(positionId);
			vertex.setTexCoord(0, texCoordId);

			model->addVertex(vertex);
		}
	}

	model::Polygon polygon;
	for (int32_t iz = 0; iz < outputSize - 1; ++iz)
	{
		int32_t offset = iz * outputSize;
		for (int32_t ix = 0; ix < outputSize - 1; ++ix)
		{
			float wx, wz;
			heightfield->gridToWorld(ix * step, iz * step, wx, wz);

			if (!heightfield->getWorldCut(wx, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz))
				continue;
			if (!heightfield->getWorldCut(wx + step, wz + step))
				continue;
			if (!heightfield->getWorldCut(wx, wz + step))
				continue;

			int32_t indices[] =
			{
				offset + ix,
				offset + ix + 1,
				offset + ix + 1 + outputSize,
				offset + ix + outputSize
			};

			polygon.clearVertices();
			polygon.addVertex(indices[0]);
			polygon.addVertex(indices[1]);
			polygon.addVertex(indices[3]);
			model->addPolygon(polygon);

			polygon.clearVertices();
			polygon.addVertex(indices[1]);
			polygon.addVertex(indices[2]);
			polygon.addVertex(indices[3]);
			model->addPolygon(polygon);
		}
	}

	return model::ModelFormat::writeAny(fileName, model);
}

	}
}
