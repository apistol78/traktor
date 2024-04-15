/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
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

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSetPipeline", 4, PointSetPipeline, editor::IPipeline)

bool PointSetPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void PointSetPipeline::destroy()
{
}

TypeInfoSet PointSetPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< PointSetAsset >();
}

bool PointSetPipeline::shouldCache() const
{
	return true;
}

uint32_t PointSetPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
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
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const PointSetAsset* pointSetAsset = checked_type_cast< const PointSetAsset* >(sourceAsset);

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + pointSetAsset->getFileName());
	Ref< model::Model > model = model::ModelFormat::readAny(filePath);
	if (!model)
	{
		log::error << L"PointSet pipeline failed; unable to read source model (" << pointSetAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< PointSet > pointSet = new PointSet();
	if (!pointSetAsset->fromFaces())
	{
		for (const auto& vertex : model->getVertices())
		{
			PointSet::Point point;

			point.position = model->getPosition(vertex.getPosition());

			if (vertex.getNormal() != model::c_InvalidIndex)
				point.normal = model->getNormal(vertex.getNormal());
			else
				point.normal = Vector4::zero();

			if (vertex.getColor() != model::c_InvalidIndex)
				point.color = model->getColor(vertex.getColor());
			else
				point.color = Vector4::one();

			pointSet->add(point);
		}
	}
	else
	{
		for (const auto& polygon : model->getPolygons())
		{
			const auto& vertices = polygon.getVertices();

			PointSet::Point point;
			point.position = Vector4::zero();
			point.normal = Vector4::zero();
			point.color = Vector4::zero();

			for (auto vi : vertices)
			{
				const model::Vertex& vertex = model->getVertex(vi);
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

Ref< ISerializable > PointSetPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
