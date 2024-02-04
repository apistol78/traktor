/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IComponentEditor.h"
#include "Scene/Editor/IComponentEditorFactory.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/IEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
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
{
}

void EntityAdapter::prepare(
	world::EntityData* entityData,
	world::Entity* entity
)
{
	m_entityData = entityData;
	
	if ((m_entity = entity) != nullptr)
	{
		if (m_visible)
			m_entity->modifyState(world::EntityState::Visible, world::EntityState::None);
		else
			m_entity->modifyState(world::EntityState::None, world::EntityState::Visible);
	}

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

void EntityAdapter::destroyEntity()
{
	safeDestroy(m_entity);
}

void EntityAdapter::setComponentProduct(const world::IEntityComponentData* componentData, world::IEntityComponent* component)
{
	const uint32_t hash = DeepHash(componentData).get();
	m_componentProducts[&type_of(componentData)] = { hash, component };
}

world::IEntityComponent* EntityAdapter::findComponentProduct(const world::IEntityComponentData* componentData)
{
	const auto it = m_componentProducts.find(&type_of(componentData));
	if (it == m_componentProducts.end())
		return nullptr;

	const uint32_t hash = DeepHash(componentData).get();
	if (it->second.hash != hash)
		return nullptr;

	return it->second.component;
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

	// Notify all ancestor component editors about the modified transform.
	for (EntityAdapter* adapter = this; adapter != nullptr; adapter = adapter->getParent())
	{
		for (auto componentEditor : adapter->m_componentEditors)
			componentEditor->transformModified(adapter, this);
	}
}

Transform EntityAdapter::getTransform0() const
{
	return m_entityData ? m_entityData->getTransform() : Transform::identity();
}

void EntityAdapter::setTransform(const Transform& transform)
{
	if (m_entityData)
		m_entityData->setTransform(transform);
	if (m_entity)
		m_entity->setTransform(transform);

	// Notify all ancestor component editors about the modified transform.
	for (EntityAdapter* adapter = this; adapter != nullptr; adapter = adapter->getParent())
	{
		for (auto componentEditor : adapter->m_componentEditors)
			componentEditor->transformModified(adapter, this);
	}
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

void EntityAdapter::addChild(EntityAdapter* insertAfter, EntityAdapter* child)
{
	if (m_entityEditor->addChildEntity(insertAfter, child))
		link(insertAfter, child);
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
		m_entityEditor->addChildEntity(nullptr, child);
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

void EntityAdapter::link(EntityAdapter* linkAfter, EntityAdapter* child)
{
	T_FATAL_ASSERT_M (child->m_parent == nullptr, L"Child already linked to another parent");
	T_FATAL_ASSERT_M (std::find(m_children.begin(), m_children.end(), child) == m_children.end(), L"Child already added");
	child->m_parent = this;

	auto it = std::find(m_children.begin(), m_children.end(), linkAfter);
	if (it != m_children.end())
		m_children.insert(it + 1, child);
	else
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
	if (m_entity)
	{
		if (m_visible)
			m_entity->modifyState(world::EntityState::Visible, world::EntityState::None);
		else
			m_entity->modifyState(world::EntityState::None, world::EntityState::Visible);
	}
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

bool EntityAdapter::isGeometry() const
{
	for (auto component : m_entityData->getComponents())
	{
		if (std::wstring(type_name(component)) == L"traktor.world.LightComponentData")
			return false;
		if (std::wstring(type_name(component)) == L"traktor.world.ProbeComponentData")
			return false;
		if (std::wstring(type_name(component)) == L"traktor.world.VolumetricFogComponentData")
			return false;
		if (std::wstring(type_name(component)) == L"traktor.weather.SkyComponentData")
			return false;
	}
	return true;
}

AlignedVector< EntityAdapter::SnapPoint > EntityAdapter::getSnapPoints() const
{
	AlignedVector< SnapPoint > snapPoints;

	const Transform transform = getTransform();
	const Aabb3 boundingBox = m_entity->getBoundingBox();
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

			faceCenter /= 4.0_simd;

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
