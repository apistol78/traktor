#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class ShaderModuleCache : public Object
{
public:
	explicit ShaderModuleCache(VkDevice logicalDevice);

	virtual ~ShaderModuleCache();

	VkShaderModule get(const AlignedVector< uint32_t >& shader, uint32_t shaderHash);

private:
	VkDevice m_logicalDevice = 0;
	SmallMap< uint32_t, VkShaderModule > m_shaderModules;
};

}
