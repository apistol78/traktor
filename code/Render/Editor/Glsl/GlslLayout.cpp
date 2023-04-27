/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslLayout.h"
#include "Render/Editor/Glsl/GlslResource.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslLayout", GlslLayout, Object)

void GlslLayout::add(GlslResource* resource)
{
	T_ASSERT(resource->m_layout == nullptr);
	m_resources.push_back(resource);

	// Note! Emitter assumes the first three resources are GlslUniformBuffer;s
	// but since they will be sorted first due to it's name it's currently safe to ignore this
	// explicit constraint.
	m_resources.sort([](const GlslResource* r0, const GlslResource* r1) -> bool {
		const wchar_t* tn0 = type_name(r0);
		const wchar_t* tn1 = type_name(r1);
		int32_t c = wcscmp(tn0, tn1);
		if (c < 0)
			return false;
		else if (c > 0)
			return true;
		if (r0->getOrdinal() >= r1->getOrdinal())
			return false;
		else
			return true;
	});

	// Calculate binding locations.
	int32_t binding = 0;
	for (auto resource : m_resources)
	{
		if (!resource->isBindless())
			resource->m_binding = binding++;
	}
}

GlslResource* GlslLayout::get(int32_t index)
{
	return m_resources[index];
}

GlslResource* GlslLayout::get(const std::wstring& name)
{
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const GlslResource* resource) {
		return resource->getName() == name;
	});
	return it != m_resources.end() ? (*it).ptr() : nullptr;
}

const GlslResource* GlslLayout::get(const std::wstring& name) const
{
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [&](const GlslResource* resource) {
		return resource->getName() == name;
	});
	return it != m_resources.end() ? (*it) : nullptr;
}

RefArray< GlslResource > GlslLayout::get(uint8_t stageMask) const
{
	RefArray< GlslResource > stageResources;
	for (auto resource : m_resources)
	{
		if ((resource->getStages() & stageMask) != 0)
			stageResources.push_back(resource);
	}
	return stageResources;
}

int32_t GlslLayout::getGlobalIndex(const GlslResource* resource) const
{
	int32_t index = 0;
	for (auto untypedResource : m_resources)
	{
		if (untypedResource == resource)
			return index;
		++index;
	}
	return -1;
}

uint32_t GlslLayout::count(const TypeInfo& resourceType, uint8_t stageMask) const
{
	uint32_t c = 0;
	for (auto resource : m_resources)
	{
		if ((resource->getStages() & stageMask) != 0 && is_type_a(resourceType, type_of(resource)))
			++c;
	}
	return c;
}

}
