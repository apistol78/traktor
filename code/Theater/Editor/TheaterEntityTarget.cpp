/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEntityTarget.h"

#include "Scene/Editor/EntityAdapter.h"
#include "Theater/TheaterEntityComponent.h"
#include "Theater/TheaterEntityComponentData.h"
#include "World/Entity.h"

namespace traktor::theater
{
namespace
{

scene::EntityAdapter* findEntityAdapterInChildren(const scene::EntityAdapter* parentAdapter, const Guid& id)
{
	for (auto childAdapter : parentAdapter->getChildren())
	{
		if (childAdapter->getId() == id)
			return childAdapter;
	}
	for (auto childAdapter : parentAdapter->getChildren())
	{
		if (scene::EntityAdapter* foundAdapter = findEntityAdapterInChildren(childAdapter, id))
			return foundAdapter;
	}
	return nullptr;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterEntityTarget", TheaterEntityTarget, TheaterEditTarget)

TheaterEntityTarget::TheaterEntityTarget(scene::EntityAdapter* ownerAdapter, TheaterEntityComponentData* componentData)
:	m_ownerAdapter(ownerAdapter)
,	m_componentData(componentData)
{
}

std::wstring TheaterEntityTarget::getName() const
{
	return m_ownerAdapter->getPath();
}

Guid TheaterEntityTarget::getId() const
{
	return m_ownerAdapter->getId();
}

RefArray< ActData >& TheaterEntityTarget::getActs()
{
	return m_componentData->getActs();
}

Transform TheaterEntityTarget::getBaseTransform() const
{
	return m_ownerAdapter->getTransform();
}

scene::EntityAdapter* TheaterEntityTarget::findEntityAdapter(const Guid& id) const
{
	return findEntityAdapterInChildren(m_ownerAdapter, id);
}

bool TheaterEntityTarget::canCapture(const scene::EntityAdapter* entityAdapter) const
{
	// Only children of the owner entity can be animated as tracks are
	// resolved within the owner's group at runtime.
	for (const scene::EntityAdapter* parentAdapter = entityAdapter != nullptr ? entityAdapter->getParent() : nullptr; parentAdapter != nullptr; parentAdapter = parentAdapter->getParent())
	{
		if (parentAdapter == m_ownerAdapter)
			return true;
	}
	return false;
}

Performance* TheaterEntityTarget::getPerformance() const
{
	TheaterEntityComponent* component = m_ownerAdapter->getComponent< TheaterEntityComponent >();
	return component != nullptr ? component->getPerformance() : nullptr;
}

void TheaterEntityTarget::rebuild()
{
	world::Entity* entity = m_ownerAdapter->getEntity();
	if (entity == nullptr)
		return;

	// Replace component of owner entity with an instance of the modified data;
	// this is much cheaper than rebuilding all entities of the scene.
	// No entity builder at hand; track events are not instantiated which is
	// fine as events are never issued while previewing in the editor.
	Ref< TheaterEntityComponent > component = m_componentData->createComponent(nullptr);
	if (!component)
		return;

	entity->setComponent(component);
	m_ownerAdapter->setComponentProduct(m_componentData, component);
}

}
