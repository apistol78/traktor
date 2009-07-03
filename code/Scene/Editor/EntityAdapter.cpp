#include <algorithm>
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter(world::EntityInstance* instance)
:	m_instance(instance)
,	m_selected(false)
{
}

world::EntityInstance* EntityAdapter::getInstance() const
{
	return m_instance;
}

world::EntityData* EntityAdapter::getEntityData() const
{
	T_ASSERT (m_instance);
	return m_instance->getEntityData();
}

void EntityAdapter::setEntity(world::Entity* entity)
{
	m_entity = entity;
}

world::Entity* EntityAdapter::getEntity() const
{
	return m_entity;
}

std::wstring EntityAdapter::getName() const
{
	return m_instance->getName();
}

std::wstring EntityAdapter::getTypeName() const
{
	return type_name(getEntityData());
}

bool EntityAdapter::isSpatial() const
{
	return is_a< world::SpatialEntityData >(getEntityData());
}

void EntityAdapter::setTransform(const Matrix44& transform)
{
	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(getEntityData()))
		spatialEntityData->setTransform(transform);
	if (world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity))
		spatialEntity->setTransform(transform);
}

Matrix44 EntityAdapter::getTransform() const
{
	if (world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity))
	{
		Matrix44 transform;
		if (spatialEntity->getTransform(transform))
			return transform;
	}

	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(getEntityData()))
		return spatialEntityData->getTransform();

	return Matrix44::identity();
}

Aabb EntityAdapter::getBoundingBox() const
{
	if (world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity))
		return spatialEntity->getBoundingBox();
	return Aabb();
}

bool EntityAdapter::isExternal() const
{
	for (const EntityAdapter* entityAdapter = this; entityAdapter; entityAdapter = entityAdapter->m_parent)
	{
		if (is_a< world::ExternalEntityData >(entityAdapter->getEntityData()) || is_a< world::ExternalSpatialEntityData >(entityAdapter->getEntityData()))
			return true;
	}
	return false;
}

bool EntityAdapter::isChildOfExternal() const
{
	if (!m_parent)
		return false;

	return m_parent->isExternal();
}

bool EntityAdapter::getExternalGuid(Guid& outGuid) const
{
	if (const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(getEntityData()))
	{
		outGuid = externalEntityData->getGuid();
		return true;
	}
	if (const world::ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const world::ExternalSpatialEntityData* >(getEntityData()))
	{
		outGuid = externalSpatialEntityData->getGuid();
		return true;
	}
	return false;
}

bool EntityAdapter::addReference(EntityAdapter* reference)
{
	T_ASSERT (m_instance);
	T_ASSERT (reference->m_instance);

	const RefArray< world::EntityInstance >& references = m_instance->getReferences();
	if (std::find(references.begin(), references.end(), reference->m_instance) != references.end())
		return false;

	m_instance->addReference(reference->m_instance);
	return true;
}

void EntityAdapter::removeReference(EntityAdapter* reference)
{
	T_ASSERT (m_instance);
	T_ASSERT (reference->m_instance);

	m_instance->removeReference(reference->m_instance);
}

bool EntityAdapter::isGroup() const
{
	return is_a< world::GroupEntityData >(getEntityData());
}

EntityAdapter* EntityAdapter::getParent() const
{
	return m_parent;
}

EntityAdapter* EntityAdapter::getParentGroup()
{
	Ref< EntityAdapter > entity = this;
	for (; entity; entity = entity->getParent())
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

EntityAdapter* EntityAdapter::getParentContainerGroup()
{
	Ref< EntityAdapter > entity = getParent();
	for (; entity; entity = entity->getParent())
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

bool EntityAdapter::addChild(EntityAdapter* child, bool modifyEntityData)
{
	T_ASSERT (child);

	if (modifyEntityData)
	{
		if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(getEntityData()))
			groupEntityData->addInstance(child->getInstance());
		else
			return false;
	}

	child->m_parent = this;
	m_children.push_back(child);

	return true;
}

void EntityAdapter::removeChild(EntityAdapter* child, bool modifyEntityData)
{
	T_ASSERT (child);
	T_ASSERT (child->m_parent == this);

	if (modifyEntityData)
	{
		if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(getEntityData()))
			groupEntityData->removeInstance(child->getInstance());
	}

	RefArray< EntityAdapter >::iterator i = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT (i != m_children.end());
	m_children.erase(i);

	child->m_parent = 0;
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

void EntityAdapter::unlink()
{
	if (m_parent)
	{
		m_parent->removeChild(this, false);
		m_parent = 0;
	}
}

void EntityAdapter::setEntityEditor(IEntityEditor* entityEditor)
{
	m_entityEditor = entityEditor;
}

IEntityEditor* EntityAdapter::getEntityEditor() const
{
	return m_entityEditor;
}

bool EntityAdapter::isSelected() const
{
	return m_selected;
}

void EntityAdapter::setHash(const MD5& hash)
{
	m_hash = hash;
}

const MD5& EntityAdapter::getHash() const
{
	return m_hash;
}

void EntityAdapter::setUserObject(Object* userObject)
{
	m_userObject = userObject;
}

Object* EntityAdapter::getUserObject() const
{
	return m_userObject;
}

AlignedVector< EntityAdapter::SnapPoint > EntityAdapter::getSnapPoints() const
{
	AlignedVector< SnapPoint > snapPoints;

	world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity);
	if (spatialEntity)
	{
		Matrix44 transform = getTransform();
		Aabb boundingBox = spatialEntity->getBoundingBox();
		if (!boundingBox.empty())
		{
			Vector4 extents[8];
			boundingBox.getExtents(extents);

			const Vector4* normals = Aabb::getNormals();
			const int* faces = Aabb::getFaces();

			for (int i = 0; i < 6; ++i)
			{
				Vector4 faceCenter =
					extents[faces[i * 4 + 0]] +
					extents[faces[i * 4 + 1]] +
					extents[faces[i * 4 + 2]] +
					extents[faces[i * 4 + 3]];

				faceCenter /= Scalar(4.0f);

				SnapPoint sp;
				sp.position = transform * faceCenter;
				sp.direction = transform * normals[i];
				snapPoints.push_back(sp);
			}
		}
	}

	return snapPoints;
}

	}
}
