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
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class RenderPassCache : public Object
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct Specification
	{
		uint8_t msaaSampleCount;
		uint8_t clear;
		uint8_t load;
		uint8_t store;
		VkFormat colorTargetFormats[RenderTargetSetCreateDesc::MaxTargets];
		VkFormat depthTargetFormat;

		uint32_t hash() const;
	};
#pragma pack()

	explicit RenderPassCache(VkDevice logicalDevice);

	bool get(
		const Specification& spec,
		VkRenderPass& outRenderPass
	);

private:
	VkDevice m_logicalDevice;
	SmallMap< uint32_t, VkRenderPass > m_renderPasses;
};

}
