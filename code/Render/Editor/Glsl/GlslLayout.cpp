/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslLayout.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslLayout", GlslLayout, Object)

void GlslLayout::addStatic(GlslResource* resource, int32_t binding)
{
	resource->m_binding = binding;
	m_staticResources.push_back(resource);
	updateResourceLayout();
}

void GlslLayout::addBindless(GlslResource* resource)
{
	resource->m_binding = -1;
	m_bindlessResources.push_back(resource);
	updateResourceLayout();
}

void GlslLayout::add(GlslResource* resource)
{
	m_dynamicResources.push_back(resource);
	m_dynamicResources.sort([](const GlslResource* r0, const GlslResource* r1) -> bool {
		// First group by class ordinal so each resource type is clustered.
		if (r0->getClassOrdinal() > r1->getClassOrdinal())
			return false;
		else if (r0->getClassOrdinal() < r1->getClassOrdinal())
			return true;

		// Then sort on resource ordinal within each group.
		if (r0->getOrdinal() >= r1->getOrdinal())
			return false;
		else
			return true;
	});

	int32_t binding = 0;
	for (auto resource : m_staticResources)
		binding = std::max(binding, resource->m_binding + 1);
	for (auto resource : m_dynamicResources)
		resource->m_binding = binding++;

	updateResourceLayout();
}

GlslResource* GlslLayout::getByIndex(int32_t index)
{
	return m_resources[index];
}

GlslResource* GlslLayout::getByBinding(int32_t binding)
{
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const GlslResource* resource) {
		return resource->getBinding() == binding;
	});
	return it != m_resources.end() ? (*it).ptr() : nullptr;
}

const GlslResource* GlslLayout::getByName(const std::wstring& name) const
{
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const GlslResource* resource) {
		return resource->getName() == name;
	});
	return it != m_resources.end() ? (*it) : nullptr;
}

GlslResource* GlslLayout::getByName(const std::wstring& name)
{
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const GlslResource* resource) {
		return resource->getName() == name;
	});
	return it != m_resources.end() ? (*it).ptr() : nullptr;
}

RefArray< GlslResource > GlslLayout::getBySet(GlslResource::Set set)
{
	RefArray< GlslResource > setResources;
	for (auto resource : m_resources)
		if (resource->getSet() == set)
			setResources.push_back(resource);
	return setResources;
}

RefArray< GlslResource > GlslLayout::getByStage(uint8_t stageMask) const
{
	RefArray< GlslResource > stageResources;
	for (auto resource : m_resources)
		if ((resource->getStages() & stageMask) != 0)
			stageResources.push_back(resource);
	return stageResources;
}

uint32_t GlslLayout::count(const TypeInfo& resourceType, uint8_t stageMask) const
{
	uint32_t c = 0;
	for (auto resource : m_resources)
		if ((resource->getStages() & stageMask) != 0 && is_type_a(resourceType, type_of(resource)))
			++c;
	return c;
}

void GlslLayout::updateResourceLayout()
{
	m_resources = m_staticResources;
	m_resources.insert(m_resources.end(), m_dynamicResources.begin(), m_dynamicResources.end());
	m_resources.insert(m_resources.end(), m_bindlessResources.begin(), m_bindlessResources.end());
}

}
