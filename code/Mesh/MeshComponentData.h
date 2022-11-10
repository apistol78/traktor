/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class Entity;

}

namespace traktor::mesh
{

class IMesh;
class MeshComponent;

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	MeshComponentData() = default;

	explicit MeshComponentData(const resource::Id< IMesh >& mesh);

	Ref< MeshComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	void setMesh(const resource::Id< IMesh >& mesh) { m_mesh = mesh; }

	const resource::Id< IMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< IMesh > m_mesh;
	bool m_screenSpaceCulling = false;
};

}
