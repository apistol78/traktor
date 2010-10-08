#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreePartition.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreePartition", OctreePartition, IPartition)

OctreePartition::OctreePartition(OctreeNode* node)
:	m_node(node)
{
}

void OctreePartition::traverse(
	const Frustum& frustum,
	const Matrix44& world,
	const Matrix44& view,
	render::handle_t worldTechnique,
	std::set< uint32_t >& outPartIndices
) const
{
	Matrix44 worldViewInv = (view * world).inverse();
	Frustum frustumObject;

	for (int32_t i = 0; i < 6; ++i)
		frustumObject.planes[i] = worldViewInv * frustum.planes[i];

	m_node->traverse(frustumObject, worldTechnique, outPartIndices);
}

	}
}
