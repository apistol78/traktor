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

class StaticMesh;

/*! Static mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS StaticMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

private:
	resource::Proxy< StaticMesh > m_mesh;
	Transform m_lastTransform;		//!< Last rendered transform.
};

}
