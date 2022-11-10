/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/MeshComponent.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

/*! Blend mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS BlendMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	BlendMeshComponent(const resource::Proxy< BlendMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	void setBlendWeights(const AlignedVector< float >& blendWeights);

	const AlignedVector< float >& getBlendWeights() const;

private:
	resource::Proxy< BlendMesh > m_mesh;
	Ref< BlendMesh::Instance > m_instance;
	AlignedVector< float > m_blendWeights;
};

}
