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
#include "Core/Math/Transform.h"
#include "Mesh/IMesh.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;

}

namespace traktor::world
{

class IWorldCulling;
class IWorldRenderPass;

}

namespace traktor::mesh
{

class IMeshParameterCallback;
class StaticMesh;

/*! Level-of-detail mesh.
 * \ingroup Mesh
 */
class T_DLLCLASS AutoLodMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	const Aabb3& getBoundingBox(float lodDistance) const;

	bool supportTechnique(float lodDistance, render::handle_t technique) const;

	void build(
		float lodDistance,
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class AutoLodMeshResource;

	float m_maxDistance = 0.0f;
	float m_cullDistance = 0.0f;
	Aabb3 m_boundingBox;
	RefArray< StaticMesh > m_lods;

	StaticMesh* getStaticMesh(float lodDistance) const;
};

}
