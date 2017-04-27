/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Lod/LodMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

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
:	MeshEntity(transform, true)
,	m_lods(lods)
,	m_lodDistance(lodDistance)
,	m_lodCullDistance(lodCullDistance)
{
}

void LodMeshEntity::setTransform(const Transform& transform)
{
	m_boundingBox = Aabb3();

	Transform invTransform = m_transform.get().inverse();
	for (RefArray< MeshEntity >::iterator i = m_lods.begin(); i != m_lods.end(); ++i)
	{
		Transform currentTransform;
		if ((*i)->getTransform(currentTransform))
		{
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			(*i)->setTransform(Tworld);

			Aabb3 childBoundingBox = (*i)->getBoundingBox();
			m_boundingBox.contain(childBoundingBox.transform(Tlocal));
		}
	}
	
	MeshEntity::setTransform(transform);
}

Aabb3 LodMeshEntity::getBoundingBox() const
{
	return m_boundingBox;
}

bool LodMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_lods[0]->supportTechnique(technique);
}

void LodMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float /*distance*/
)
{
	const Vector4& eyePosition = worldRenderView.getEyePosition();
	float lodDistance = (m_transform.get().translation() - eyePosition).length();

	if (m_lodCullDistance >= FUZZY_EPSILON && lodDistance >= m_lodCullDistance)
		return;

	int32_t lod = clamp< int32_t >(int32_t(lodDistance / m_lodDistance), 0, int32_t(m_lods.size()) - 1);
	worldContext.build(worldRenderView, worldRenderPass, m_lods[lod]);
}

void LodMeshEntity::update(const world::UpdateParams& update)
{
	for (RefArray< MeshEntity >::iterator i = m_lods.begin(); i != m_lods.end(); ++i)
		(*i)->update(update);
}

	}
}
