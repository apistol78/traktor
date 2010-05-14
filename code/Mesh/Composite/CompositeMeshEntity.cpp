#include "Mesh/Composite/CompositeMeshEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.CompositeMeshEntity", CompositeMeshEntity, MeshEntity)

CompositeMeshEntity::CompositeMeshEntity(const Transform& transform)
:	MeshEntity(transform)
{
}

void CompositeMeshEntity::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * m_transform.inverse();
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Transform currentTransform;
		if (i->second->getTransform(currentTransform))
			i->second->setTransform(deltaTransform * currentTransform);
	}
	m_transform = transform;
}

Aabb CompositeMeshEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb boundingBox;
	for (std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Aabb childBoundingBox = i->second->getBoundingBox();
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

void CompositeMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		i->second->setUserParameter(m_userParameter);
		worldContext->build(worldRenderView, i->second);
	}
	m_first = false;
}

void CompositeMeshEntity::update(const world::EntityUpdate* update)
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

Ref< MeshEntity > CompositeMeshEntity::getMeshEntity(const std::wstring& name) const
{
	std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.find(name);
	return i != m_meshEntities.end() ? i->second.ptr() : 0;
}

	}
}
