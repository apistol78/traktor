/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.CompositeMeshEntity", CompositeMeshEntity, MeshEntity)

CompositeMeshEntity::CompositeMeshEntity(const Transform& transform)
:	MeshEntity(transform, true)
{
}

void CompositeMeshEntity::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.get().inverse();
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Transform currentTransform;
		if (i->second->getTransform(currentTransform))
		{
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			i->second->setTransform(Tworld);
		}
	}
	MeshEntity::setTransform(transform);
}

Aabb3 CompositeMeshEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.get().inverse();

	Aabb3 boundingBox;
	for (std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Aabb3 childBoundingBox = i->second->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform;
			i->second->getTransform(childTransform);

			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

bool CompositeMeshEntity::supportTechnique(render::handle_t technique) const
{
	for (std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		if (i->second->supportTechnique(technique))
			return true;
	}
	return false;
}

void CompositeMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
		worldContext.build(worldRenderView, worldRenderPass, i->second);
}

void CompositeMeshEntity::update(const world::UpdateParams& update)
{
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
		i->second->update(update);
}

void CompositeMeshEntity::addMeshEntity(const std::wstring& name, MeshEntity* meshEntity)
{
	T_ASSERT_M (!name.empty(), L"Composite mesh entities must have a name");
	T_ASSERT_M (m_meshEntities.find(name) == m_meshEntities.end(), L"Composite mesh entities must have a unique name");
	m_meshEntities[name] = meshEntity;
}

MeshEntity* CompositeMeshEntity::getMeshEntity(const std::wstring& name) const
{
	std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.find(name);
	return i != m_meshEntities.end() ? i->second.ptr() : 0;
}

	}
}
