#include "Amalgam/Editor/Prefab/PrefabMerge.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.PrefabMerge", 0, PrefabMerge, ISerializable)

PrefabMerge::PrefabMerge(bool partitionMesh)
:	m_partitionMesh(partitionMesh)
{
}

void PrefabMerge::setName(const std::wstring& name)
{
	m_name = name;
}

void PrefabMerge::addVisualMesh(const mesh::MeshAsset* visualMeshAsset, const Transform& transform)
{
	VisualMesh vm = { visualMeshAsset, transform };
	m_visualMeshes.push_back(vm);
}

void PrefabMerge::addShapeMesh(const physics::MeshAsset* shapeMeshAsset, const Transform& transform)
{
	ShapeMesh sm = { shapeMeshAsset, transform };
	m_shapeMeshes.push_back(sm);
}

void PrefabMerge::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< bool >(L"partitionMesh", m_partitionMesh);
	s >> MemberAlignedVector< VisualMesh, MemberComposite< VisualMesh > >(L"visualMeshes", m_visualMeshes);
	s >> MemberAlignedVector< ShapeMesh, MemberComposite< ShapeMesh > >(L"shapeMeshes", m_shapeMeshes);
}

void PrefabMerge::VisualMesh::serialize(ISerializer& s)
{
	s >> MemberRef< const mesh::MeshAsset >(L"meshAsset", meshAsset);
	s >> MemberComposite< Transform >(L"transform", transform);
}

void PrefabMerge::ShapeMesh::serialize(ISerializer& s)
{
	s >> MemberRef< const physics::MeshAsset >(L"meshAsset", meshAsset);
	s >> MemberComposite< Transform >(L"transform", transform);
}

	}
}
