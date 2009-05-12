#include "Mesh/Composite/CompositeMeshEntity.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.CompositeMeshEntity", CompositeMeshEntity, MeshEntity)

CompositeMeshEntity::CompositeMeshEntity(const Matrix44& transform)
:	MeshEntity(transform)
{
}

void CompositeMeshEntity::setTransform(const Matrix44& transform)
{
	Matrix44 deltaTransform = m_transform.inverseOrtho() * transform;
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Matrix44 currentTransform;
		if (i->second->getTransform(currentTransform))
			i->second->setTransform(currentTransform * deltaTransform);
	}
	m_transform = transform;
}

Aabb CompositeMeshEntity::getBoundingBox() const
{
	Matrix44 invTransform = m_transform.inverseOrtho();

	Aabb boundingBox;
	for (std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
	{
		Aabb childBoundingBox = i->second->getWorldBoundingBox();
		if (!childBoundingBox.empty())
			boundingBox.contain(childBoundingBox.transform(invTransform));
	}

	return boundingBox;
}

void CompositeMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	for (std::map< std::wstring, Ref< MeshEntity > >::iterator i = m_meshEntities.begin(); i != m_meshEntities.end(); ++i)
		worldContext->render(worldRenderView, i->second);
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

MeshEntity* CompositeMeshEntity::getMeshEntity(const std::wstring& name) const
{
	std::map< std::wstring, Ref< MeshEntity > >::const_iterator i = m_meshEntities.find(name);
	return i != m_meshEntities.end() ? i->second.getPtr() : 0;
}

	}
}
