#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Static/StaticMesh.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.AutoLodMesh", AutoLodMesh, IMesh)

AutoLodMesh::AutoLodMesh()
:	m_maxDistance(0.0f)
,	m_cullDistance(0.0f)
{
}

const Aabb3& AutoLodMesh::getBoundingBox(float lodDistance) const
{
	return m_boundingBox;
}

bool AutoLodMesh::supportTechnique(float lodDistance, render::handle_t technique) const
{
	StaticMesh* staticMesh = getStaticMesh(lodDistance);
	if (staticMesh)
		return staticMesh->supportTechnique(technique);
	else
		return true;
}

void AutoLodMesh::render(
	float lodDistance,
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const IntervalTransform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	StaticMesh* staticMesh = getStaticMesh(lodDistance);
	if (staticMesh)
		staticMesh->render(
			renderContext,
			worldRenderPass,
			worldTransform,
			distance,
			parameterCallback
		);
}

StaticMesh* AutoLodMesh::getStaticMesh(float lodDistance) const
{
	if (lodDistance >= m_cullDistance)
		return 0;

	int32_t lastLod = int32_t(m_lods.size()) - 1;
	int32_t index = clamp(int32_t(lodDistance * lastLod / m_maxDistance), 0, lastLod);

	return m_lods[index];
}

	}
}
