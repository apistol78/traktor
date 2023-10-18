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

class Context;

class PipelineLayoutCache : public Object
{
public:
	explicit PipelineLayoutCache(Context* context);

	virtual ~PipelineLayoutCache();

	bool get(
		uint32_t pipelineHash,
		bool useTargetSize,
		const VkDescriptorSetLayoutCreateInfo& dlci,
		VkDescriptorSetLayout& outDescriptorSetLayout,
		VkPipelineLayout& outPipelineLayout
	);

	VkSampler getSampler(const VkSamplerCreateInfo& sci);

private:
	struct Entry
	{
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
#if defined(_DEBUG)
		std::wstring debug;
#endif
	};

	Context* m_context;
	SmallMap< uint32_t, Entry > m_entries;
	SmallMap< uint32_t, VkSampler > m_samplers;
};

}