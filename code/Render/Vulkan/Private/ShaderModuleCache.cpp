/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/ShaderModuleCache.h"

namespace traktor::render
{

ShaderModuleCache::ShaderModuleCache(VkDevice logicalDevice)
:	m_logicalDevice(logicalDevice)
{
}

ShaderModuleCache::~ShaderModuleCache()
{
	for (auto& it : m_shaderModules)
		vkDestroyShaderModule(m_logicalDevice, it.second, 0);
}

VkShaderModule ShaderModuleCache::get(const AlignedVector< uint32_t >& shader, uint32_t shaderHash)
{
	VkShaderModule sm = 0;
	const auto it = m_shaderModules.find(shaderHash);
	if (it != m_shaderModules.end())
		sm = it->second;
	else
	{
		VkShaderModuleCreateInfo vsmci = {};
		vsmci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vsmci.codeSize = shader.size() * sizeof(uint32_t);
		vsmci.pCode = shader.c_ptr();
		if (vkCreateShaderModule(m_logicalDevice, &vsmci, nullptr, &sm) != VK_SUCCESS)
			return 0;
		m_shaderModules[shaderHash] = sm;
	}
	return sm;
}

}
