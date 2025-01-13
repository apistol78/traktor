/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/ShaderModuleCache.h"

#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor::render
{

ShaderModuleCache::ShaderModuleCache(VkDevice logicalDevice)
	: m_logicalDevice(logicalDevice)
{
}

ShaderModuleCache::~ShaderModuleCache()
{
	for (auto& it : m_shaderModules)
		vkDestroyShaderModule(m_logicalDevice, it.second, 0);
}

VkShaderModule ShaderModuleCache::get(const AlignedVector< uint32_t >& shader, uint32_t shaderHash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	VkShaderModule sm = 0;
	const auto it = m_shaderModules.find(shaderHash);
	if (it != m_shaderModules.end())
		sm = it->second;
	else
	{
		const VkShaderModuleCreateInfo vsmci = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = shader.size() * sizeof(uint32_t),
			.pCode = shader.c_ptr()
		};

		if (vkCreateShaderModule(m_logicalDevice, &vsmci, nullptr, &sm) != VK_SUCCESS)
			return 0;

		m_shaderModules[shaderHash] = sm;
	}
	return sm;
}

}
