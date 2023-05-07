/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Mesh/MeshResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class StaticMeshResource;

/*! Automatic lod mesh persistent resource.
 * \ingroup Mesh
 */
class T_DLLCLASS AutoLodMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class AutoLodMeshConverter;

	float m_maxDistance = 0.0f;
	float m_cullDistance = 0.0f;
	Aabb3 m_boundingBox;
	RefArray< StaticMeshResource > m_lods;
};

}
