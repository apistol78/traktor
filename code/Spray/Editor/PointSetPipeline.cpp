#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Spray/PointSet.h"
#include "Spray/PointSetResource.h"
#include "Spray/Editor/PointSetAsset.h"
#include "Spray/Editor/PointSetPipeline.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSetPipeline", 4, PointSetPipeline, editor::IPipeline)

bool PointSetPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void PointSetPipeline::destroy()
{
}

TypeInfoSet PointSetPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PointSetAsset >());
	return typeSet;
}

bool PointSetPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const PointSetAsset* pointSetAsset = checked_type_cast< const PointSetAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), pointSetAsset->getFileName().getOriginal());
	return true;
}

bool PointSetPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const PointSetAsset* pointSetAsset = checked_type_cast< const PointSetAsset* >(sourceAsset);

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), pointSetAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"PointSet pipeline failed; unable to open source model (" << pointSetAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< model::Model > model = model::ModelFormat::readAny(
		file,
		pointSetAsset->getFileName().getExtension(),
		model::ModelFormat::IfMeshPositions | model::ModelFormat::IfMeshVertices | model::ModelFormat::IfMeshPolygons
	);
	if (!model)
	{
		log::error << L"PointSet pipeline failed; unable to read source model (" << pointSetAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< PointSet > pointSet = new PointSet();
	if (!pointSetAsset->fromFaces())
	{
		const std::vector< model::Vertex >& vertices = model->getVertices();
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

			pointSet->add(point);
		}
	}
	else
	{
		const std::vector< model::Polygon >& polygons = model->getPolygons();
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

			pointSet->add(point);
		}
	}

	log::info << L"PointSet created, " << uint32_t(pointSet->get().size()) << L" point(s)" << Endl;

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
		return false;

	instance->setObject(new PointSetResource());
	
	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		instance->revert();
		return false;
	}
		
	if (!pointSet->write(stream))
	{
		instance->revert();
		return false;
	}

	return instance->commit();
}

Ref< ISerializable > PointSetPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
