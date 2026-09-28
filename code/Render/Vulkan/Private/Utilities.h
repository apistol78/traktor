/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

/*! Pick the first of \a candidates supported as an optimal tiling depth/stencil attachment.
 * Also requires sampled image support if \a usedAsTexture; VK_FORMAT_UNDEFINED if none is supported.
 */
VkFormat determineSupportedDepthTargetFormat(VkPhysicalDevice physicalDevice, const VkFormat* candidates, int32_t candidateCount, bool usedAsTexture);

/*! Pipeline stages which can access an image in the given layout. */
VkPipelineStageFlags getPipelineStageFlags(const VkImageLayout layout);

/*! Access types an image in the given layout can be accessed with. */
VkAccessFlags getAccessMask(const VkImageLayout layout);

/*! Restrict a stage mask to the stages of a queue without graphics; an empty result is replaced with \a fallback. */
VkPipelineStageFlags restrictToComputeStages(VkPipelineStageFlags stages, VkPipelineStageFlags fallback);

/*! Restrict an access mask to the access types of a queue without graphics. */
VkAccessFlags restrictToComputeAccess(VkAccessFlags access);

}

