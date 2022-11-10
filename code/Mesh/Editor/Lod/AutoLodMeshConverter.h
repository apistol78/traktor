/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/Editor/IMeshConverter.h"

namespace traktor
{
	namespace mesh
	{

class AutoLodMeshConverter : public IMeshConverter
{
public:
	virtual Ref< MeshResource > createResource() const override final;

	virtual bool getOperations(const MeshAsset* meshAsset, RefArray< const model::IModelOperation >& outOperations) const override final;

	virtual bool convert(
		const MeshAsset* meshAsset,
		const RefArray< model::Model >& models,
		const Guid& materialGuid,
		const std::map< std::wstring, std::list< MeshMaterialTechnique > >& materialTechniqueMap,
		const AlignedVector< render::VertexElement >& vertexElements,
		int32_t maxInstanceCount,
		MeshResource* meshResource,
		IStream* meshResourceStream
	) const override final;
};

	}
}

