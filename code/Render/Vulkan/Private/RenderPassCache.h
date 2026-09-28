/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Core/Thread/CriticalSection.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

#include <cstring>

namespace traktor::render
{

/*! Cache of render passes, shared by every view; render passes are never evicted.
 * \ingroup Render
 */
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

		bool operator < (const Specification& rh) const { return std::memcmp(this, &rh, sizeof(Specification)) < 0; }

		bool operator > (const Specification& rh) const { return std::memcmp(this, &rh, sizeof(Specification)) > 0; }

		bool operator == (const Specification& rh) const { return std::memcmp(this, &rh, sizeof(Specification)) == 0; }
	};
#pragma pack()

	explicit RenderPassCache(VkDevice logicalDevice);

	virtual ~RenderPassCache();

	bool get(
		const Specification& spec,
		VkRenderPass& outRenderPass
	);

private:
	VkDevice m_logicalDevice;
	CriticalSection m_lock;
	SmallMap< Specification, VkRenderPass > m_renderPasses;
};

}
