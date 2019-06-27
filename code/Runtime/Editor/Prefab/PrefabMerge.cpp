#include "Runtime/Editor/Prefab/PrefabMerge.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.PrefabMerge", PrefabMerge, Object)

void PrefabMerge::addVisualMesh(const mesh::MeshAsset* visualMeshAsset, const Transform& transform)
{
	m_visualMeshes.push_back({ visualMeshAsset, transform });
}

void PrefabMerge::addShapeMesh(const physics::MeshAsset* shapeMeshAsset, const Transform& transform)
{
	m_shapeMeshes.push_back({ shapeMeshAsset, transform });
}

	}
}
