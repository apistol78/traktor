/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Core/Timer/Profiler.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/VertexLayoutVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/UniformBufferPool.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Context", Context, Object)

Context::Context(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	uint32_t graphicsQueueIndex,
	uint32_t computeQueueIndex
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_graphicsQueueIndex(graphicsQueueIndex)
,	m_computeQueueIndex(computeQueueIndex)
,	m_sampledResourceIndexAllocator(0, MaxBindlessResources - 1)
,	m_storageResourceIndexAllocator(0, MaxBindlessResources - 1)
,	m_bufferResourceIndexAllocator(0, MaxBindlessResources - 1)
{
}

Context::~Context()
{
	// Destroy pipelines.
	for (auto& pipeline : m_pipelines)
		vkDestroyPipeline(m_logicalDevice, pipeline.second.pipeline, nullptr);
	m_pipelines.clear();

	// Destroy uniform buffer pools.
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
	{
		m_uniformBufferPools[i]->destroy();
		m_uniformBufferPools[i] = nullptr;
	}

	// Destroy descriptor pool.
	if (m_descriptorPool != 0)
	{
		vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
		m_descriptorPool = 0;
	}
}

bool Context::create()
{
	AlignedVector< uint8_t > buffer;

	// Create queues.
	m_graphicsQueue = Queue::create(this, m_graphicsQueueIndex);
	m_computeQueue = Queue::create(this, m_computeQueueIndex);

	// Create pipeline cache.
	VkPipelineCacheCreateInfo pcci = {};
	pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pcci.flags = 0;
	pcci.initialDataSize = 0;
	pcci.pInitialData = nullptr;

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmRead);
	if (file)
	{
	 	const uint32_t size = (uint32_t)file->available();
		buffer.resize(size);
	 	file->read(buffer.ptr(), size);
	 	file->close();

		pcci.initialDataSize = size;
		pcci.pInitialData = buffer.c_ptr();

		log::debug << L"Pipeline cache \"" << ss.str() << L"\" loaded successfully." << Endl;
	}
	else
		log::debug << L"No pipeline cache found; creating new cache." << Endl;

	vkCreatePipelineCache(
		m_logicalDevice,
		&pcci,
		nullptr,
		&m_pipelineCache
	);

	// Create descriptor set pool.
	VkDescriptorPoolSize dps[6];
	dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	dps[0].descriptorCount = 80000;
	dps[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	dps[1].descriptorCount = 80000;
	dps[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	dps[2].descriptorCount = MaxBindlessResources;
	dps[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	dps[3].descriptorCount = 8000;
	dps[4].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	dps[4].descriptorCount = MaxBindlessResources;
	dps[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	dps[5].descriptorCount = MaxBindlessResources;

	const VkDescriptorPoolCreateInfo dpci =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
		.maxSets = 32000,
		.poolSizeCount = sizeof_array(dps),
		.pPoolSizes = dps
	};

	vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &m_descriptorPool);

	// Create uniform buffer pools.
	m_uniformBufferPools[0] = new UniformBufferPool(this,   1000, L"Once");
	m_uniformBufferPools[1] = new UniformBufferPool(this,  10000, L"Frame");
	m_uniformBufferPools[2] = new UniformBufferPool(this, 100000, L"Draw");

	// Bindless resources.
	const uint32_t bindings[] = { BindlessTexturesBinding, BindlessImagesBinding, BindlessBuffersBinding };
	VkDescriptorType descriptorTypes[] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER };
	VkDescriptorSetLayout* layouts[] = { &m_bindlessTexturesDescriptorLayout, &m_bindlessImagesDescriptorLayout, &m_bindlessBuffersDescriptorLayout };
	VkDescriptorSet* descriptorSets[] = { &m_bindlessTexturesDescriptorSet, &m_bindlessImagesDescriptorSet, &m_bindlessBuffersDescriptorSet };

	for (int32_t i = 0; i < sizeof_array(bindings); ++i)
	{
		const VkDescriptorBindingFlags bindlessFlags = 
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;/* |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;*/

		const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
			.bindingCount = 1,
			.pBindingFlags = &bindlessFlags
		};

		const VkDescriptorSetLayoutBinding binding =
		{
			.binding = bindings[i],
			.descriptorType = descriptorTypes[i],
			.descriptorCount = MaxBindlessResources,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		};

		const VkDescriptorSetLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &extendedInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			.bindingCount = 1,
			.pBindings = &binding
		};

		if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, layouts[i]) != VK_SUCCESS)
		{
			log::error << L"Failed to create Vulkan; failed to create bindless descriptor layout." << Endl;
			return false;
		}

		// Create descriptor set.
		const uint32_t maxBinding = MaxBindlessResources - 1;

		const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &maxBinding	// This number is the max allocatable count.
		};

		const VkDescriptorSetAllocateInfo allocInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &countInfo,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = layouts[i]
		};

		VkResult result;
		if ((result = vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, descriptorSets[i])) != VK_SUCCESS)
		{
			log::error << L"Failed to create Vulkan; failed to create bindless descriptor set. " << getHumanResult(result) << Endl;
			return false;

		}
	}

	return true;
}

void Context::incrementViews()
{
	Atomic::increment(m_views);
}

void Context::decrementViews()
{
	Atomic::decrement(m_views);
}

void Context::addDeferredCleanup(const cleanup_fn_t& fn, uint32_t cleanupFlags)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

	// In case there are no render views which can perform cleanup
	// after each frame, we do this immediately.
	if (m_views > 0)
		m_cleanupFns.push_back({ fn, cleanupFlags });
	else
		fn(this);
}

void Context::addCleanupListener(ICleanupListener* cleanupListener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	m_cleanupListeners.push_back(cleanupListener);
}

void Context::removeCleanupListener(ICleanupListener* cleanupListener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);
	auto it = std::find(m_cleanupListeners.begin(), m_cleanupListeners.end(), cleanupListener);
	if (it != m_cleanupListeners.end())
		m_cleanupListeners.erase(it);
}

void Context::performCleanup()
{
	if (m_cleanupFns.empty())
		return;

	{
		T_PROFILER_SCOPE(L"Context::performCleanup");

		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_graphicsQueue->m_lock);
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_cleanupLock);

		bool gpuIdle = false;
		bool freeDescriptors = false;
		while (!m_cleanupFns.empty())
		{
			// Take over vector in case more resources are added for cleanup from callbacks.
			AlignedVector< DeferredCleanup > cleanupFns;
			cleanupFns.swap(m_cleanupFns);

			// Invoke cleanups, flush GPU if necessary.
			for (const DeferredCleanup& cleanupFn : cleanupFns)
			{
				if (!gpuIdle && (cleanupFn.flags & CleanupNeedFlushGPU) != 0)
				{
					vkDeviceWaitIdle(m_logicalDevice);
					gpuIdle = true;
				}
				freeDescriptors |= (bool)((cleanupFn.flags & CleanupFreeDescriptorSets) != 0);
				cleanupFn.fn(this);
			}
		}

		// Only call cleanup listeners to free descriptors.
		if (freeDescriptors)
		{
			for (auto cleanupListener : m_cleanupListeners)
				cleanupListener->postCleanup();
		}
	}
}

void Context::recycle()
{
	for (int32_t i = 0; i < sizeof_array(m_uniformBufferPools); ++i)
		m_uniformBufferPools[i]->recycle();
}

bool Context::savePipelineCache()
{
	size_t size = 0;
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, nullptr);
	if (!size)
		return true;

	AlignedVector< uint8_t > buffer(size, 0);
	vkGetPipelineCacheData(m_logicalDevice, m_pipelineCache, &size, buffer.ptr());

	StringOutputStream ss;
#if defined(__IOS__)
	ss << OS::getInstance().getUserHomePath() << L"/Library/Caches/Traktor/Vulkan/Pipeline.cache";
#else
	ss << OS::getInstance().getWritableFolderPath() << L"/Traktor/Vulkan/Pipeline.cache";
#endif

	FileSystem::getInstance().makeAllDirectories(Path(ss.str()).getPathOnly());

	Ref< IStream > file = FileSystem::getInstance().open(ss.str(), File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to save pipeline cache; failed to create file \"" << ss.str() << L"\"." << Endl;
		return false;
	}

	file->write(buffer.c_ptr(), size);
	file->close();
	
	log::debug << L"Pipeline cache \"" << ss.str() << L"\" saved successfully." << Endl;
	return true;
}

uint32_t Context::allocateSampledResourceIndex()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_sampledResourceIndexAllocator.alloc();
}

void Context::freeSampledResourceIndex(uint32_t resourceIndex)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_sampledResourceIndexAllocator.free(resourceIndex);
}

uint32_t Context::allocateStorageResourceIndex(uint32_t span)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_storageResourceIndexAllocator.allocSequential(span);
}

void Context::freeStorageResourceIndex(uint32_t resourceIndex, uint32_t span)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_storageResourceIndexAllocator.freeSequential(resourceIndex, span);
}

uint32_t Context::allocateBufferResourceIndex()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	return m_bufferResourceIndexAllocator.alloc();
}

void Context::freeBufferResourceIndex(uint32_t resourceIndex)
{
	T_FATAL_ASSERT(resourceIndex != ~0U);
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_resourceIndexLock);
	m_bufferResourceIndexAllocator.free(resourceIndex);
}



VkPipeline Context::validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt, uint32_t targetRenderPassHash, const RenderTargetSetVk* targetSet, VkRenderPass targetRenderPass, float multiSampleShading)
{
	// Calculate pipeline key.
	const uint8_t primitiveId = (uint8_t)pt;
	const uint32_t declHash = (vertexLayout != nullptr) ? vertexLayout->getHash() : 0;
	const uint32_t shaderHash = program->getShaderHash();
	const auto key = std::make_tuple(primitiveId, targetRenderPassHash, declHash, shaderHash);

	VkPipeline pipeline = 0;

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
	{
		it->second.lastAcquired = /*m_counter*/0;
		pipeline = it->second.pipeline;
	}
	else
	{
		const RenderState& rs = program->getRenderState();
		const uint32_t colorAttachmentCount = targetSet->getColorTargetCount();

		const VkViewport vp = {
			.width = 1,
			.height = 1,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		const VkRect2D sc = {
			.offset = { 0, 0 },
			.extent = { 65536, 65536 }
		};

		const VkPipelineViewportStateCreateInfo vsci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = &vp,
			.scissorCount = 1,
			.pScissors = &sc
		};

		VkPipelineVertexInputStateCreateInfo visci = {};
		visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (vertexLayout != nullptr)
		{
			visci.vertexBindingDescriptionCount = 1;
			visci.pVertexBindingDescriptions = &vertexLayout->getVkVertexInputBindingDescription();
			visci.vertexAttributeDescriptionCount = (uint32_t)vertexLayout->getVkVertexInputAttributeDescriptions().size();
			visci.pVertexAttributeDescriptions = vertexLayout->getVkVertexInputAttributeDescriptions().c_ptr();
		}
		else
		{
			visci.vertexBindingDescriptionCount = 0;
			visci.pVertexBindingDescriptions = nullptr;
			visci.vertexAttributeDescriptionCount = 0;
			visci.pVertexAttributeDescriptions = nullptr;
		}

		StaticVector< VkPipelineShaderStageCreateInfo, 2 > ssci;
		ssci.push_back({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = program->getVertexVkShaderModule(),
			.pName = "main",
			.pSpecializationInfo = nullptr });
		ssci.push_back({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = program->getFragmentVkShaderModule(),
			.pName = "main",
			.pSpecializationInfo = nullptr });

		const VkPipelineRasterizationStateCreateInfo rsci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = rs.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
			.cullMode = (VkCullModeFlags)c_cullMode[(int32_t)rs.cullMode],
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0,
			.depthBiasClamp = 0,
			.depthBiasSlopeFactor = 0,
			.lineWidth = 1
		};

		VkPipelineMultisampleStateCreateInfo mssci = {};
		mssci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mssci.rasterizationSamples = targetSet->getVkSampleCount();
		if (multiSampleShading > FUZZY_EPSILON)
		{
			mssci.sampleShadingEnable = VK_TRUE;
			mssci.minSampleShading = multiSampleShading;
		}
		else
			mssci.sampleShadingEnable = VK_FALSE;
		mssci.pSampleMask = nullptr;
		mssci.alphaToCoverageEnable = rs.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
		mssci.alphaToOneEnable = VK_FALSE;

		const VkStencilOpState sops = {
			.failOp = c_stencilOperations[(int)rs.stencilFail],
			.passOp = c_stencilOperations[(int)rs.stencilPass],
			.depthFailOp = c_stencilOperations[(int)rs.stencilZFail],
			.compareOp = c_compareOperations[(int)rs.stencilFunction],
			.compareMask = rs.stencilMask,
			.writeMask = rs.stencilMask,
			.reference = rs.stencilReference
		};

		const VkPipelineDepthStencilStateCreateInfo dssci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = rs.depthEnable ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = rs.depthWriteEnable ? VK_TRUE : VK_FALSE,
			.depthCompareOp = rs.depthEnable ? c_compareOperations[(int)rs.depthFunction] : VK_COMPARE_OP_ALWAYS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = rs.stencilEnable ? VK_TRUE : VK_FALSE,
			.front = sops,
			.back = sops,
			.minDepthBounds = 0,
			.maxDepthBounds = 0
		};

		StaticVector< VkPipelineColorBlendAttachmentState, RenderTargetSetCreateDesc::MaxTargets > blendAttachments;
		for (uint32_t i = 0; i < colorAttachmentCount; ++i)
		{
			auto& cbas = blendAttachments.push_back();
			cbas.blendEnable = rs.blendEnable ? VK_TRUE : VK_FALSE;
			cbas.srcColorBlendFactor = c_blendFactors[(int)rs.blendColorSource];
			cbas.dstColorBlendFactor = c_blendFactors[(int)rs.blendColorDestination];
			cbas.colorBlendOp = c_blendOperations[(int)rs.blendColorOperation];
			cbas.srcAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaSource];
			cbas.dstAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaDestination];
			cbas.alphaBlendOp = c_blendOperations[(int)rs.blendAlphaOperation];
			cbas.colorWriteMask = rs.colorWriteMask;
		}

		const VkPipelineColorBlendStateCreateInfo cbsci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_CLEAR,
			.attachmentCount = (uint32_t)blendAttachments.size(),
			.pAttachments = blendAttachments.c_ptr(),
			.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
		};

		const VkDynamicState ds[3] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_STENCIL_REFERENCE };
		const VkPipelineDynamicStateCreateInfo dsci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = rs.stencilEnable ? 3U : 2U,
			.pDynamicStates = ds
		};

		const VkPipelineInputAssemblyStateCreateInfo iasci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = c_primitiveTopology[(int32_t)pt],
			.primitiveRestartEnable = VK_FALSE
		};

		const VkGraphicsPipelineCreateInfo gpci = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = (uint32_t)ssci.size(),
			.pStages = ssci.c_ptr(),
			.pVertexInputState = &visci,
			.pInputAssemblyState = &iasci,
			.pTessellationState = nullptr,
			.pViewportState = &vsci,
			.pRasterizationState = &rsci,
			.pMultisampleState = &mssci,
			.pDepthStencilState = &dssci,
			.pColorBlendState = &cbsci,
			.pDynamicState = &dsci,
			.layout = program->getPipelineLayout(),
			.renderPass = targetRenderPass,
			.subpass = 0,
			.basePipelineHandle = 0,
			.basePipelineIndex = 0
		};

		const VkResult result = vkCreateGraphicsPipelines(
			m_logicalDevice,
			m_pipelineCache,
			1,
			&gpci,
			nullptr,
			&pipeline);
		if (result != VK_SUCCESS)
		{
#if defined(_DEBUG)
			log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L"), \"" << program->getTag() << L"\"." << Endl;
#else
			log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
			return 0;
		}

		m_pipelines[key] = { /*m_counter*/0, pipeline };
#if defined(_DEBUG)
		log::debug << L"Graphics pipeline created (" << program->getTag() << L", " << m_pipelines.size() << L" pipelines)." << Endl;
#endif
	}

	return pipeline;
}

VkPipeline Context::validateComputePipeline(const ProgramVk* p)
{
	// Calculate pipeline key.
	const uint8_t primitiveId = 0;
	const uint32_t declHash = 0;
	const uint32_t shaderHash = p->getShaderHash();
	const auto key = std::make_tuple(primitiveId, 0, declHash, shaderHash);

	VkPipeline pipeline = 0;

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
	{
		it->second.lastAcquired = 0/*m_counter*/;
		pipeline = it->second.pipeline;
	}
	else
	{
		const VkPipelineShaderStageCreateInfo ssci = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = p->getComputeVkShaderModule(),
			.pName = "main",
			.pSpecializationInfo = nullptr
		};

		const VkComputePipelineCreateInfo cpci = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = ssci,
			.layout = p->getPipelineLayout()
		};

		const VkResult result = vkCreateComputePipelines(
			m_logicalDevice,
			m_pipelineCache,
			1,
			&cpci,
			nullptr,
			&pipeline);
		if (result != VK_SUCCESS)
		{
#if defined(_DEBUG)
			log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L"), \"" << p->getTag() << L"\"." << Endl;
#else
			log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
			return 0;
		}

		m_pipelines[key] = { 0/*m_counter*/, pipeline };
#if defined(_DEBUG)
		log::debug << L"Compute pipeline created (" << p->getTag() << L", " << m_pipelines.size() << L" pipelines)." << Endl;
#endif
	}

	return pipeline;
}

}
