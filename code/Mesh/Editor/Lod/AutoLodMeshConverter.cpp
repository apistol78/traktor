/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/Lod/AutoLodMeshConverter.h"
#include "Mesh/Editor/Static/StaticMeshConverter.h"
#include "Mesh/Lod/AutoLodMeshResource.h"
#include "Mesh/Static/StaticMeshResource.h"
#include "Model/Model.h"
#include "Model/Operations/Reduce.h"

namespace traktor
{
	namespace mesh
	{

Ref< MeshResource > AutoLodMeshConverter::createResource() const
{
	return new AutoLodMeshResource();
}

bool AutoLodMeshConverter::getOperations(const MeshAsset* meshAsset, RefArray< const model::IModelOperation >& outOperations) const
{
	return StaticMeshConverter().getOperations(meshAsset, outOperations);
}

bool AutoLodMeshConverter::convert(
	const MeshAsset* meshAsset,
	const RefArray< model::Model >& models,
	const Guid& materialGuid,
	const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
	const AlignedVector< render::VertexElement >& vertexElements,
	int32_t maxInstanceCount,
	MeshResource* meshResource,
	IStream* meshResourceStream
) const
{
	AutoLodMeshResource* lodMeshResource = checked_type_cast< AutoLodMeshResource* >(meshResource);

	lodMeshResource->m_boundingBox = models[0]->getBoundingBox();
	lodMeshResource->m_maxDistance = meshAsset->getLodMaxDistance();
	lodMeshResource->m_cullDistance = meshAsset->getLodCullDistance();

	for (int32_t lod = 0; lod < meshAsset->getLodSteps(); ++lod)
	{
		log::info << L"Generating detail model, level " << lod << L"..." << Endl;

		model::Model lodModel = *models[0];

		if (lod > 0)
		{
			float detailLevel = 1.0f - lod / float(meshAsset->getLodSteps());
			model::Reduce(detailLevel).apply(lodModel);
		}

		StaticMeshConverter staticMeshConverter;

		Ref< MeshResource > staticMeshResource = staticMeshConverter.createResource();

		RefArray< model::Model > lodModels(1);
		lodModels[0] = &lodModel;

		staticMeshConverter.convert(
			meshAsset,
			lodModels,
			materialGuid,
			materialTechniqueMap,
			vertexElements,
			maxInstanceCount,
			staticMeshResource,
			meshResourceStream
		);

		lodMeshResource->m_lods.push_back(checked_type_cast< StaticMeshResource* >(staticMeshResource));
	}

	return true;
}

	}
}
