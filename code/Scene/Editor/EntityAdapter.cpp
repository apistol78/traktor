#include <algorithm>
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityEditor.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/Entity.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/SpatialGroupEntityData.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter(world::EntityData* entityData)
:	m_entityData(entityData)
,	m_entityDataHash(DeepHash(entityData).getMD5())
,	m_selected(false)
,	m_modified(false)
{
}

world::EntityData* EntityAdapter::getEntityData() const
{
	return m_entityData;
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
	return m_entityData->getName();
}

std::wstring EntityAdapter::getTypeName() const
{
	return type_name(m_entityData);
}

bool EntityAdapter::isSpatial() const
{
	return is_a< world::SpatialEntityData >(m_entityData);
}

void EntityAdapter::setTransform(const Matrix44& transform)
{
	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(m_entityData))
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

	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(m_entityData))
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
		if (is_a< world::ExternalEntityData >(entityAdapter->m_entityData) || is_a< world::ExternalSpatialEntityData >(entityAdapter->m_entityData))
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
	if (const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(m_entityData))
	{
		outGuid = externalEntityData->getGuid();
		return true;
	}
	if (const world::ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const world::ExternalSpatialEntityData* >(m_entityData))
	{
		outGuid = externalSpatialEntityData->getGuid();
		return true;
	}
	return false;
}

bool EntityAdapter::isGroup() const
{
	return is_a< world::GroupEntityData >(m_entityData) || is_a< world::SpatialGroupEntityData >(m_entityData);
}

void EntityAdapter::setParent(EntityAdapter* parent)
{
	m_parent = parent;
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
		if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(m_entityData))
			groupEntityData->addEntityData(child->m_entityData);
		else if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(m_entityData))
		{
			if (world::SpatialEntityData* childEntityData = dynamic_type_cast< world::SpatialEntityData* >(child->m_entityData))
				spatialGroupEntityData->addEntityData(childEntityData);
			else
				return false;
		}
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
		if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(m_entityData))
			groupEntityData->removeEntityData(child->m_entityData);
		else if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(m_entityData))
			spatialGroupEntityData->removeEntityData(checked_type_cast< world::SpatialEntityData* >(child->m_entityData));
	}

	RefArray< EntityAdapter >::iterator i = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT (i != m_children.end());

	m_children.erase(i);
}

void EntityAdapter::removeFromParent()
{
	T_ASSERT (m_parent);

	m_parent->removeChild(this, false);
	m_parent = 0;
}

void EntityAdapter::removeAllChildren()
{
	// Recursively remove all children to ensure all links are broken.
	for (RefArray< EntityAdapter >::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->setParent(0);
		(*i)->removeAllChildren();
	}
	m_children.resize(0);
}

void EntityAdapter::setChildren(const RefArray< EntityAdapter >& children)
{
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
		(*i)->m_parent = this;
	m_children = children;
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

void EntityAdapter::setEntityEditor(EntityEditor* entityEditor)
{
	m_entityEditor = entityEditor;
}

EntityEditor* EntityAdapter::getEntityEditor() const
{
	return m_entityEditor;
}

bool EntityAdapter::isSelected() const
{
	return m_selected;
}

void EntityAdapter::updateModified()
{
	m_modified |= bool(DeepHash(m_entityData).getMD5() != m_entityDataHash);
}

void EntityAdapter::forceModified()
{
	m_modified = true;
}

void EntityAdapter::resetModified()
{
	m_modified = false;
}

bool EntityAdapter::isModified() const
{
	if (m_modified)
		return true;

	for (RefArray< EntityAdapter >::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		if ((*i)->isModified())
			return true;
	}

	return false;
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
