/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/Cloth.h"
#include "Animation/Editor/Cloth/ClothAsset.h"
#include "Animation/Editor/Cloth/ClothPipeline.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothPipeline", 16, ClothPipeline, editor::IPipeline)

bool ClothPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

void ClothPipeline::destroy()
{
}

TypeInfoSet ClothPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ClothAsset >();
}

uint32_t ClothPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ClothPipeline::shouldCache() const
{
	return true;
}

bool ClothPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const ClothAsset > clothAsset = mandatory_non_null_type_cast< const ClothAsset* >(sourceAsset);
	pipelineDepends->addDependency(m_assetPath, clothAsset->getFileName().getPathName());
	return true;
}

bool ClothPipeline::buildOutput(
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
	Ref< const ClothAsset > clothAsset = mandatory_non_null_type_cast< const ClothAsset* >(sourceAsset);

	// Read source model.
	const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + clothAsset->getFileName());
	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, L"");
	if (!model)
	{
		log::error << L"Unable to build cloth; no such file \"" << clothAsset->getFileName().getPathName() << L"\"." << Endl;
		return false;
	}

	Ref< Cloth > cloth = new Cloth();

	cloth->m_nodes.reserve(model->getPositionCount());
	for (const auto& position : model->getPositions())
	{
		Cloth::Node& n = cloth->m_nodes.push_back();
		n.position = position;
		n.east = -1;
		n.north = -1;
	}

	for (const auto& polygon : model->getPolygons())
	{
		if (polygon.getVertexCount() == 2)
		{
			const Vector4 pa = model->getVertexPosition(polygon.getVertex(0));
			const Vector4 pb = model->getVertexPosition(polygon.getVertex(1));
			const Scalar ln = (pb - pa).length();

			Cloth::Edge& e = cloth->m_edges.push_back();
			e.indices[0] = model->getVertex(polygon.getVertex(0)).getPosition();
			e.indices[1] = model->getVertex(polygon.getVertex(1)).getPosition();
			e.length = ln;
		}
		else if (polygon.getVertexCount() == 3)
		{
			for (int32_t i = 0; i < 3; ++i)
			{
				cloth->m_triangles.push_back(
					model->getVertex(polygon.getVertex(i)).getPosition()
				);
			}
		}
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!instance)
	{
		log::error << L"Unable to build cloth; unable to create output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	instance->setObject(cloth);

	if (!instance->commit())
	{
		log::error << L"Unable to build cloth; unable to commit output instance \"" << outputPath << L"\"." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ClothPipeline::buildProduct(
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
