#include "Mesh/Lod/LodMeshEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.LodMeshEntity", LodMeshEntity, MeshEntity)

LodMeshEntity::LodMeshEntity(
	const Transform& transform,
	const RefArray< MeshEntity >& lods,
	float lodDistance,
	float lodCullDistance
)
:	MeshEntity(transform)
,	m_lods(lods)
,	m_lodDistance(lodDistance)
,	m_lodCullDistance(lodCullDistance)
{
}

void LodMeshEntity::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * m_transform[1].inverse();
	for (RefArray< MeshEntity >::iterator i = m_lods.begin(); i != m_lods.end(); ++i)
	{
		Transform currentTransform;
		if ((*i)->getTransform(currentTransform))
			(*i)->setTransform(deltaTransform * currentTransform);
	}
	MeshEntity::setTransform(transform);
}

Aabb3 LodMeshEntity::getBoundingBox() const
{
	Transform invTransform = m_transform[0].inverse();

	Aabb3 boundingBox;
	for (RefArray< MeshEntity >::const_iterator i = m_lods.begin(); i != m_lods.end(); ++i)
	{
		Aabb3 childBoundingBox = (*i)->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform;
			(*i)->getTransform(childTransform);

			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

bool LodMeshEntity::supportTechnique(render::handle_t technique) const
{
	return (!m_lods.empty()) ? m_lods[0]->supportTechnique(technique) : false;
}

void LodMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (m_lods.empty())
		return;

	if (m_lodCullDistance >= FUZZY_EPSILON && distance >= m_lodCullDistance)
		return;

	int32_t lod = clamp< int32_t >(int32_t(distance / m_lodDistance), 0, m_lods.size() - 1);
	
	m_lods[lod]->setUserParameter(m_userParameter);
	worldContext.build(worldRenderView, worldRenderPass, m_lods[lod]);
}

void LodMeshEntity::update(const world::EntityUpdate* update)
{
	for (RefArray< MeshEntity >::iterator i = m_lods.begin(); i != m_lods.end(); ++i)
		(*i)->update(update);
}

	}
}
