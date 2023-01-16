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
#include "Core/Containers/AlignedVector.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class SkinnedMesh;

}

namespace traktor::physics
{

class PhysicsManager;

}

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

class EntityData;
class IEntityBuilder;

}

namespace traktor::animation
{

class AnimatedMeshComponent;

/*! Animated mesh entity data.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	AnimatedMeshComponentData() = default;

	explicit AnimatedMeshComponentData(
		const resource::Id< mesh::SkinnedMesh >& mesh
	);

	Ref< AnimatedMeshComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< mesh::SkinnedMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< mesh::SkinnedMesh > m_mesh;
	bool m_screenSpaceCulling = true;
};

}
