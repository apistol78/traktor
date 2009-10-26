#include "Spray/Editor/PointSetPipeline.h"
#include "Spray/Editor/PointSetAsset.h"
#include "Spray/PointSet.h"
#include "Model/Formats/ModelFormat.h"
#include "Model/Model.h"
#include "Editor/IPipelineManager.h"
#include "Editor/Settings.h"
#include "Database/Instance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.PointSetPipeline", PointSetPipeline, editor::IPipeline)

bool PointSetPipeline::create(const editor::Settings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void PointSetPipeline::destroy()
{
}

uint32_t PointSetPipeline::getVersion() const
{
	return 1;
}

TypeSet PointSetPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< PointSetAsset >());
	return typeSet;
}

bool PointSetPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const PointSetAsset* pointSetAsset = checked_type_cast< const PointSetAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, pointSetAsset->getFileName());
	pipelineManager->addDependency(fileName);
	return true;
}

bool PointSetPipeline::buildOutput(
	editor::IPipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const PointSetAsset* pointSetAsset = checked_type_cast< const PointSetAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, pointSetAsset->getFileName());

	// Import source model.
	Ref< model::Model > model = model::ModelFormat::readAny(fileName, model::ModelFormat::IfMesh);
	if (!model)
	{
		log::error << L"PointSet pipeline failed; unable to read source model (" << fileName.getPathName() << L")" << Endl;
		return false;
	}

	AlignedVector< PointSet::Point > points;

	if (!pointSetAsset->fromFaces())
	{
		const std::vector< model::Vertex >& vertices = model->getVertices();
		points.reserve(vertices.size());

		for (std::vector< model::Vertex >::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		{
			PointSet::Point point;

			point.position = model->getPosition(i->getPosition());

			if (i->getNormal() != model::c_InvalidIndex)
				point.normal = model->getNormal(i->getNormal());
			else
				point.normal = Vector4::zero();

			if (i->getColor() != model::c_InvalidIndex)
				point.color = model->getColor(i->getColor());
			else
				point.color = Vector4::one();

			points.push_back(point);
		}
	}
	else
	{
		const std::vector< model::Polygon >& polygons = model->getPolygons();
		points.reserve(polygons.size());

		for (std::vector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			const std::vector< uint32_t >& vertices = i->getVertices();

			PointSet::Point point;
			point.position = Vector4::zero();
			point.normal = Vector4::zero();
			point.color = Vector4::zero();

			for (std::vector< uint32_t >::const_iterator j = vertices.begin(); j != vertices.end(); ++j)
			{
				const model::Vertex& vertex = model->getVertex(*j);

				point.position += model->getPosition(vertex.getPosition());

				if (vertex.getNormal() != model::c_InvalidIndex)
					point.normal += model->getNormal(vertex.getNormal());

				if (vertex.getColor() != model::c_InvalidIndex)
					point.color += model->getColor(vertex.getColor());
			}

			Scalar norm(1.0f / vertices.size());

			point.position = (point.position * norm).xyz1();
			point.normal = (point.normal * norm).xyz0();
			point.color = (point.color * norm).xyz1();

			points.push_back(point);
		}
	}

	Ref< PointSet > pointSet = gc_new< PointSet >(cref(points));

	Ref< db::Instance > instance = pipelineManager->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
		return false;

	instance->setObject(pointSet);

	return instance->commit();
}

	}
}
