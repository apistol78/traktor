/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.mesh.MeshComponentData", 1, MeshComponentData, world::IEntityComponentData)

MeshComponentData::MeshComponentData(const resource::Id< IMesh >& mesh)
:	m_mesh(mesh)
{
}

Ref< MeshComponent > MeshComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< IMesh > mesh;
	if (!resourceManager->bind(m_mesh, mesh))
		return nullptr;

	Ref< MeshComponent > component;

	if (is_a< InstanceMesh >(mesh.getResource()))
		component = new InstanceMeshComponent(resource::Proxy< InstanceMesh >(mesh.getHandle()));
	else if (is_a< SkinnedMesh >(mesh.getResource()))
		component = new SkinnedMeshComponent(resource::Proxy< SkinnedMesh >(mesh.getHandle()), renderSystem);
	else if (is_a< StaticMesh >(mesh.getResource()))
		component = new StaticMeshComponent(resource::Proxy< StaticMesh >(mesh.getHandle()));

	return component;
}

int32_t MeshComponentData::getOrdinal() const
{
	return 3000;
}

void MeshComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void MeshComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IMesh >(L"mesh", m_mesh);

	if (s.getVersion< MeshComponentData >() < 1)
	{
		bool screenSpaceCulling;
		s >> Member< bool >(L"screenSpaceCulling", screenSpaceCulling);
	}
}

}
