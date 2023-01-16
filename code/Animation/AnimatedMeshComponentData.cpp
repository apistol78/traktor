/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/IPoseControllerData.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimatedMeshComponentData", 2, AnimatedMeshComponentData, world::IEntityComponentData)

AnimatedMeshComponentData::AnimatedMeshComponentData(
	const resource::Id< mesh::SkinnedMesh >& mesh
)
:	m_mesh(mesh)
{
}

Ref< AnimatedMeshComponent > AnimatedMeshComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, physics::PhysicsManager* physicsManager, const world::IEntityBuilder* entityBuilder) const
{
	resource::Proxy< mesh::SkinnedMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return nullptr;

	return new AnimatedMeshComponent(
		Transform::identity(),
		mesh,
		renderSystem,
		m_screenSpaceCulling
	);
}

int32_t AnimatedMeshComponentData::getOrdinal() const
{
	return 0;
}

void AnimatedMeshComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void AnimatedMeshComponentData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< AnimatedMeshComponentData >() >= 2);

	s >> resource::Member< mesh::SkinnedMesh >(L"mesh", m_mesh);
	s >> Member< bool >(L"screenSpaceCulling", m_screenSpaceCulling);
}

}
