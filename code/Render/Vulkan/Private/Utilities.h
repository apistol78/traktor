/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

// Mappings between our enums and Vulkan.
extern const VkCullModeFlagBits c_cullMode[];
extern const VkCompareOp c_compareOperations[];
extern const VkStencilOp c_stencilOperations[];
extern const VkBlendFactor c_blendFactors[];
extern const VkBlendOp c_blendOperations[];
extern const VkPrimitiveTopology c_primitiveTopology[];
extern const VkFilter c_filters[];
extern const VkSamplerMipmapMode c_mipMapModes[];
extern const VkSamplerAddressMode c_addressModes[];
extern const VkFormat c_vkTextureFormats[];
extern const VkFormat c_vkTextureFormats_sRGB[];
extern const VkFormat c_vkVertexElementFormats[];

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements);

std::wstring getHumanResult(VkResult result);

VkFormat determineSupportedTargetFormat(VkPhysicalDevice physicalDevice, TextureFormat textureFormat, bool sRGB);

/*! Pick the first depth (or depth/stencil) format from \a candidates that the
 * device supports as a depth/stencil attachment with optimal tiling. Pass
 * \a usedAsTexture to additionally require sampled-image support.
 *
 * Returns VK_FORMAT_UNDEFINED if no candidate is supported (should not happen
 * for the depth formats mandated by the Vulkan spec).
 */
VkFormat determineSupportedDepthTargetFormat(VkPhysicalDevice physicalDevice, const VkFormat* candidates, int32_t candidateCount, bool usedAsTexture);

VkPipelineStageFlags getPipelineStageFlags(const VkImageLayout layout);

VkAccessFlags getAccessMask(const VkImageLayout layout);

}

