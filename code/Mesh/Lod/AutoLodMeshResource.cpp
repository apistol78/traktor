/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshResource.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshResource.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.AutoLodMeshResource", 0, AutoLodMeshResource, IMeshResource)

AutoLodMeshResource::AutoLodMeshResource()
:	m_maxDistance(0.0f)
,	m_cullDistance(0.0f)
{
}

Ref< IMesh > AutoLodMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< AutoLodMesh > lodMesh = new AutoLodMesh();

	lodMesh->m_maxDistance = m_maxDistance;
	lodMesh->m_cullDistance = m_cullDistance;
	lodMesh->m_boundingBox = m_boundingBox;
	lodMesh->m_lods.resize(m_lods.size());

	for (uint32_t i = 0; i < m_lods.size(); ++i)
	{
		lodMesh->m_lods[i] = checked_type_cast< StaticMesh*, false >(m_lods[i]->createMesh(
			name,
			dataStream,
			resourceManager,
			renderSystem,
			meshFactory
		));
	}

	return lodMesh;
}

void AutoLodMeshResource::serialize(ISerializer& s)
{
	s >> Member< float >(L"maxDistance", m_maxDistance);
	s >> Member< float >(L"cullDistance", m_cullDistance);
	s >> MemberAabb3(L"boundingBox", m_boundingBox);
	s >> MemberRefArray< StaticMeshResource >(L"lods", m_lods);
}

	}
}
