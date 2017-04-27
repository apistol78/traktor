/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Math/Const.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IComponentEditor.h"
#include "Scene/Editor/IComponentEditorFactory.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter(SceneEditorContext* context)
:	m_context(context)
,	m_parent(0)
,	m_selected(false)
,	m_expanded(false)
,	m_visible(true)
,	m_locked(false)
,	m_hash(0)
{
}

void EntityAdapter::prepare(
	world::EntityData* entityData,
	world::Entity* entity,
	uint32_t hash
)
{
	m_entityData = entityData;
	m_entity = entity;
	m_hash = hash;

	// Create entity editor; assume type of entity data is same if already created.
	if (!m_entityEditor)
	{
		const IEntityEditorFactory* f = m_context->findEntityEditorFactory(type_of(entityData));
		T_FATAL_ASSERT (f);

		m_entityEditor = f->createEntityEditor(m_context, this);
		T_FATAL_ASSERT (m_entityEditor != 0);
	}

	// Create component editors.
	if (const world::ComponentEntityData* componentEntityData = dynamic_type_cast< const world::ComponentEntityData* >(m_entityData))
	{
		m_componentEditors.resize(0);

		const RefArray< world::IEntityComponentData >& componentData = componentEntityData->getComponents();
		for (RefArray< world::IEntityComponentData >::const_iterator i = componentData.begin(); i != componentData.end(); ++i)
		{
			const IComponentEditorFactory* f = m_context->findComponentEditorFactory(type_of(*i));
			T_FATAL_ASSERT (f);

			Ref< IComponentEditor > componentEditor = f->createComponentEditor(m_context, this, *i);
			T_FATAL_ASSERT (componentEditor);

			m_componentEditors.push_back(componentEditor);
		}
	}
}

world::EntityData* EntityAdapter::getEntityData() const
{
	return m_entityData;
}

world::Entity* EntityAdapter::getEntity() const
{
	return m_entity;
}

world::IEntityComponentData* EntityAdapter::getComponentData(const TypeInfo& componentDataType) const
{
	world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(m_entityData);
	if (componentEntityData)
		return componentEntityData->getComponent(componentDataType);
	else
		return 0;
}

world::IEntityComponent* EntityAdapter::getComponent(const TypeInfo& componentType) const
{
	world::ComponentEntity* componentEntity = dynamic_type_cast< world::ComponentEntity* >(m_entity);
	if (componentEntity)
		return componentEntity->getComponent(componentType);
	else
		return 0;
}

uint32_t EntityAdapter::getHash() const
{
	return m_hash;
}

std::wstring EntityAdapter::getName() const
{
	return m_entityData ? m_entityData->getName() : L"< Null >";
}

std::wstring EntityAdapter::getPath() const
{
	return m_parent ? m_parent->getPath() + L"/" + getName() : getName();
}

std::wstring EntityAdapter::getTypeName() const
{
	return m_entityData ? type_name(m_entityData) : L"< void >";
}

void EntityAdapter::setTransform0(const Transform& transform)
{
	if (m_entityData)
		m_entityData->setTransform(transform);
}

Transform EntityAdapter::getTransform0() const
{
	return m_entityData ? m_entityData->getTransform() : Transform::identity();
}

void EntityAdapter::setTransform(const Transform& transform)
{
	setTransform0(transform);
	if (m_entity)
		m_entity->setTransform(transform);
}

Transform EntityAdapter::getTransform() const
{
	Transform transform;
	if (m_entity && m_entity->getTransform(transform))
		return transform;
	else
		return getTransform0();
}

Aabb3 EntityAdapter::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb3();
}

bool EntityAdapter::isExternal() const
{
	for (const EntityAdapter* entityAdapter = this; entityAdapter; entityAdapter = entityAdapter->m_parent)
	{
		if (is_a< world::ExternalEntityData >(entityAdapter->getEntityData()))
			return true;
	}
	return false;
}

bool EntityAdapter::isChildOfExternal() const
{
	return m_parent ? m_parent->isExternal() : false;
}

bool EntityAdapter::getExternalGuid(Guid& outGuid) const
{
	if (const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(m_entityData))
	{
		outGuid = externalEntityData->getEntityData();
		return true;
	}
	return false;
}

bool EntityAdapter::isLayer() const
{
	return is_a< world::LayerEntityData >(m_entityData);
}

bool EntityAdapter::isChildrenPrivate() const
{
	return m_entityEditor ? m_entityEditor->isChildrenPrivate() : false;
}

bool EntityAdapter::isPrivate() const
{
	EntityAdapter* entity = m_parent;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isChildrenPrivate())
			return true;
	}
	return false;
}

bool EntityAdapter::isGroup() const
{
	return m_entityEditor ? m_entityEditor->isGroup() : false;
}

EntityAdapter* EntityAdapter::getParent() const
{
	return m_parent;
}

EntityAdapter* EntityAdapter::getParentGroup()
{
	EntityAdapter* entity = this;
	for (; entity; entity = entity->m_parent)
	{
		if (entity->isGroup())
			break;
	}
	return entity;
}

EntityAdapter* EntityAdapter::getParentContainerGroup()
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
	if (m_entityEditor->addChildEntity(child))
		link(child);
}

void EntityAdapter::removeChild(EntityAdapter* child)
{
	if (m_entityEditor->removeChildEntity(child))
		unlinkChild(child);
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

EntityAdapter* EntityAdapter::findChildAdapterFromEntity(const world::Entity* entity) const
{
	SmallMap< const world::Entity*, EntityAdapter* >::const_iterator i = m_childMap.find(entity);
	return i != m_childMap.end() ? i->second : 0;
}

void EntityAdapter::link(EntityAdapter* child)
{
	T_FATAL_ASSERT_M (child->m_parent == 0, L"Child already linked to another parent");
	T_FATAL_ASSERT_M (std::find(m_children.begin(), m_children.end(), child) == m_children.end(), L"Child already added");
	child->m_parent = this;
	m_children.push_back(child);
	m_childMap[child->getEntity()] = child;
}

void EntityAdapter::unlinkChild(EntityAdapter* child)
{
	T_FATAL_ASSERT (child);
	T_FATAL_ASSERT_M (child->m_parent == this, L"Entity adapter not child if this");

	RefArray< EntityAdapter >::iterator i = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT (i != m_children.end());

	m_children.erase(i);
	m_childMap.remove(child->getEntity());

	child->m_parent = 0;
}

void EntityAdapter::unlinkAllChildren()
{
	while (!m_children.empty())
		unlinkChild(m_children.front());
}

void EntityAdapter::unlinkFromParent()
{
	if (m_parent)
		m_parent->unlinkChild(this);
	T_FATAL_ASSERT (m_parent == 0);
}

IEntityEditor* EntityAdapter::getEntityEditor() const
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

bool EntityAdapter::isVisible(bool includingParents) const
{
	if (!m_visible)
		return false;

	if (includingParents)
	{
		for (EntityAdapter* parent = m_parent; parent; parent = parent->m_parent)
		{
			if (!parent->m_visible)
				return false;
		}
	}

	return true;
}

void EntityAdapter::setLocked(bool locked)
{
	m_locked = locked;
	if (m_locked)
		m_selected = false;
}

bool EntityAdapter::isLocked(bool includingParents) const
{
	if (m_locked)
		return true;

	if (includingParents)
	{
		for (EntityAdapter* parent = m_parent; parent; parent = parent->m_parent)
		{
			if (parent->m_locked)
				return true;
		}
	}

	return false;
}

AlignedVector< EntityAdapter::SnapPoint > EntityAdapter::getSnapPoints() const
{
	AlignedVector< SnapPoint > snapPoints;

	Transform transform = getTransform();
	Aabb3 boundingBox = m_entity->getBoundingBox();
	if (!boundingBox.empty())
	{
		Vector4 extents[8];
		boundingBox.getExtents(extents);

		const Vector4* normals = Aabb3::getNormals();
		const int* faces = Aabb3::getFaces();

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

	return snapPoints;
}

void EntityAdapter::drawGuides(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (!isVisible(true))
		return;

	if (m_entityEditor)
		m_entityEditor->drawGuide(primitiveRenderer);

	for (RefArray< IComponentEditor >::const_iterator i = m_componentEditors.begin(); i != m_componentEditors.end(); ++i)
		(*i)->drawGuide(primitiveRenderer);
}

	}
}
