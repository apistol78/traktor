/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

/*!
 * \ingroup Render
 */
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
