/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/IVertexLayout.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class VertexLayoutVk : public IVertexLayout
{
	T_RTTI_CLASS;

public:
	explicit VertexLayoutVk(
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
		int32_t positionElementIndex,
		uint32_t hash
	);

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const { return m_vertexBindingDescription; }

	const AlignedVector< VkVertexInputAttributeDescription >& getVkVertexInputAttributeDescriptions() const { return m_vertexAttributeDescriptions; }

	int32_t getPositionElementIndex() const { return m_positionElementIndex; }

	uint32_t getHash() const { return m_hash; }

private:
	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
	int32_t m_positionElementIndex;
	uint32_t m_hash;
};

}
