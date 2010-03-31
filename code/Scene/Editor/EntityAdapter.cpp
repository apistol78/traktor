#include <algorithm>
#include "Core/Math/Const.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/SpatialGroupEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter(world::EntityData* entityData)
:	m_entityData(entityData)
,	m_parent(0)
,	m_selected(false)
,	m_expanded(false)
,	m_visible(true)
{
}

Ref< world::EntityData > EntityAdapter::getEntityData() const
{
	return m_entityData;
}

void EntityAdapter::setEntity(world::Entity* entity)
{
	m_entity = entity;
}

Ref< world::Entity > EntityAdapter::getEntity() const
{
	return m_entity;
}

std::wstring EntityAdapter::getName() const
{
	return m_entityData->getName();
}

std::wstring EntityAdapter::getTypeName() const
{
	return type_name(getEntityData());
}

bool EntityAdapter::isSpatial() const
{
	return is_a< world::SpatialEntityData >(getEntityData());
}

void EntityAdapter::setTransform(const Transform& transform)
{
	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(getEntityData()))
		spatialEntityData->setTransform(transform);
	if (world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity))
		spatialEntity->setTransform(transform);
}

Transform EntityAdapter::getTransform() const
{
	if (world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity))
	{
		Transform transform;
		if (spatialEntity->getTransform(transform))
			return transform;
	}

	if (world::SpatialEntityData* spatialEntityData = dynamic_type_cast< world::SpatialEntityData* >(getEntityData()))
		return spatialEntityData->getTransform();

	return Transform::identity();
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

bool EntityAdapter::isGroup() const
{
	if (is_a< world::GroupEntityData >(getEntityData()))
		return true;
	if (is_a< world::SpatialGroupEntityData >(getEntityData()))
		return true;
	else
		return false;
}

EntityAdapter* EntityAdapter::getParent() const
{
	return m_parent;
}

Ref< EntityAdapter > EntityAdapter::getParentGroup()
{
	EntityAdapter* entity = this;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

Ref< EntityAdapter > EntityAdapter::getParentContainerGroup()
{
	EntityAdapter* entity = m_parent;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

void EntityAdapter::addChild(EntityAdapter* child)
{
	if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(getEntityData()))
		groupEntityData->addEntityData(child->getEntityData());
	else if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(getEntityData()))
	{
		if (world::SpatialEntityData* spatialChildEntityData = dynamic_type_cast< world::SpatialEntityData* >(child->getEntityData()))
			spatialGroupEntityData->addEntityData(spatialChildEntityData);
	}
	else
		T_FATAL_ERROR;

	link(child);
}

void EntityAdapter::removeChild(EntityAdapter* child)
{
	if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(getEntityData()))
		groupEntityData->removeEntityData(child->getEntityData());
	else if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(getEntityData()))
	{
		if (world::SpatialEntityData* spatialChildEntityData = dynamic_type_cast< world::SpatialEntityData* >(child->getEntityData()))
			spatialGroupEntityData->removeEntityData(spatialChildEntityData);
	}
	else
		T_FATAL_ERROR;

	unlink(child);
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

void EntityAdapter::link(EntityAdapter* child)
{
	T_ASSERT_M (child->m_parent == 0, L"Child already linked to another parent");
	child->m_parent = this;
	m_children.push_back(child);
}

void EntityAdapter::unlink(EntityAdapter* child)
{
	T_ASSERT (child);
	T_ASSERT_M (child->m_parent == this, L"Entity adapter not child if this");

	RefArray< EntityAdapter >::iterator i = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT (i != m_children.end());
	m_children.erase(i);

	child->m_parent = 0;
}

void EntityAdapter::setEntityEditor(IEntityEditor* entityEditor)
{
	m_entityEditor = entityEditor;
}

Ref< IEntityEditor > EntityAdapter::getEntityEditor() const
{
	return m_entityEditor;
}

bool EntityAdapter::isSelected() const
{
	return m_selected;
}

void EntityAdapter::setExpanded(bool expanded)
{
	m_expanded = expanded;
}

bool EntityAdapter::isExpanded() const
{
	return m_expanded;
}

void EntityAdapter::setVisible(bool visible)
{
	m_visible = visible;
}

bool EntityAdapter::isVisible() const
{
	if (!m_visible)
		return false;

	for (EntityAdapter* parent = m_parent; parent; parent = parent->m_parent)
	{
		if (!parent->m_visible)
			return false;
	}

	return true;
}

AlignedVector< EntityAdapter::SnapPoint > EntityAdapter::getSnapPoints() const
{
	AlignedVector< SnapPoint > snapPoints;

	world::SpatialEntity* spatialEntity = dynamic_type_cast< world::SpatialEntity* >(m_entity);
	if (spatialEntity)
	{
		Transform transform = getTransform();
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
