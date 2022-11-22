/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityAdapter", EntityAdapter, Object)

EntityAdapter::EntityAdapter(SceneEditorContext* context)
:	m_context(context)
,	m_entityDataType(nullptr)
,	m_parent(nullptr)
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

	// If entity data type is different then ensure we re-create editors.
	if (m_entityDataType != &type_of(m_entityData))
	{
		m_entityEditor = nullptr;
		m_componentEditors.resize(0);
		m_entityDataType = &type_of(m_entityData);
	}

	// Create entity editor; assume type of entity data is same if already created.
	if (!m_entityEditor)
	{
		const IEntityEditorFactory* factory = m_context->findEntityEditorFactory(type_of(entityData));
		T_FATAL_ASSERT (factory);

		m_entityEditor = factory->createEntityEditor(m_context, this);
		T_FATAL_ASSERT (m_entityEditor != nullptr);
	}

	// Create component editors.
	m_componentEditors.resize(0);
	for (auto componentData : entityData->getComponents())
	{
		const IComponentEditorFactory* factory = m_context->findComponentEditorFactory(type_of(componentData));
		T_FATAL_ASSERT (factory);

		Ref< IComponentEditor > componentEditor = factory->createComponentEditor(m_context, this, componentData);
		T_FATAL_ASSERT (componentEditor);

		m_componentEditors.push_back(componentEditor);
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

RefArray< world::IEntityComponent > EntityAdapter::getComponents() const
{
	return m_entity->getComponents();
}

world::IEntityComponentData* EntityAdapter::getComponentData(const TypeInfo& componentDataType) const
{
	return m_entityData->getComponent(componentDataType);
}

world::IEntityComponent* EntityAdapter::getComponent(const TypeInfo& componentType) const
{
	return m_entity->getComponent(componentType);
}

void EntityAdapter::dropHash()
{
	m_hash = 0;
}

uint32_t EntityAdapter::getHash() const
{
	return m_hash;
}

const Guid& EntityAdapter::getId() const
{
	return m_entityData ? m_entityData->getId() : Guid::null;
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
	return m_entity ? m_entity->getTransform() : getTransform0();
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

const world::ILayerAttribute* EntityAdapter::getLayerAttribute(const TypeInfo& attributeType) const
{
	if (const world::LayerEntityData* layerData = dynamic_type_cast< const world::LayerEntityData* >(m_entityData))
	{
		auto attribute = layerData->getAttribute(attributeType);
		if (attribute != nullptr)
			return attribute;
	}
	return m_parent ? m_parent->getLayerAttribute(attributeType) : nullptr;
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

void EntityAdapter::swapChildren(EntityAdapter* child1, EntityAdapter* child2)
{
	auto it1 = std::find(m_children.begin(), m_children.end(), child1);
	auto it2 = std::find(m_children.begin(), m_children.end(), child2);
	if (it1 == m_children.end() || it2 == m_children.end())
		return;

	for (auto child : m_children)
		m_entityEditor->removeChildEntity(child);

	Ref< EntityAdapter > tmp = *it1;
	*it1 = *it2;
	*it2 = tmp;

	for (auto child : m_children)
		m_entityEditor->addChildEntity(child);
}

const RefArray< EntityAdapter >& EntityAdapter::getChildren() const
{
	return m_children;
}

EntityAdapter* EntityAdapter::findChildAdapterFromEntity(const world::Entity* entity) const
{
	auto i = m_childMap.find(entity);
	return i != m_childMap.end() ? i->second : nullptr;
}

void EntityAdapter::link(EntityAdapter* child)
{
	T_FATAL_ASSERT_M (child->m_parent == nullptr, L"Child already linked to another parent");
	T_FATAL_ASSERT_M (std::find(m_children.begin(), m_children.end(), child) == m_children.end(), L"Child already added");
	child->m_parent = this;
	m_children.push_back(child);
	m_childMap[child->getEntity()] = child;
}

void EntityAdapter::unlinkChild(EntityAdapter* child)
{
	T_FATAL_ASSERT (child);
	T_FATAL_ASSERT_M (child->m_parent == this, L"Entity adapter not child if this");

	auto it = std::find(m_children.begin(), m_children.end(), child);
	T_ASSERT(it != m_children.end());

	m_children.erase(it);
	m_childMap.remove(child->getEntity());

	child->m_parent = nullptr;
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
	T_FATAL_ASSERT (m_parent == nullptr);
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

bool EntityAdapter::isDynamic() const
{
	auto editorAttributes = m_entityData->getComponent< world::EditorAttributesComponentData >();
	if (editorAttributes && editorAttributes->dynamic)
		return true;

	if (m_parent != nullptr)
		return m_parent->isDynamic();

	return false;
}

bool EntityAdapter::isPrefab() const
{
	for (auto component : m_entityData->getComponents())
	{
		if (std::wstring(type_name(component)) == L"traktor.shape.PrefabComponentData")
			return true;
	}

	if (m_parent != nullptr)
		return m_parent->isPrefab();

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
	if (!isVisible() || isLocked())
		return;

	if (m_entityEditor)
		m_entityEditor->drawGuide(primitiveRenderer);

	for (auto componentEditor : m_componentEditors)
		componentEditor->drawGuide(primitiveRenderer);
}

}
