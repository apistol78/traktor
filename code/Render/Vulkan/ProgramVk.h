/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticVector.h"
#include "Render/IProgram.h"
#include "Render/Vulkan/Private/ApiHeader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/UniformBufferPool.h"

namespace traktor::render
{

class Buffer;
class BufferViewVk;
class CommandBuffer;
class PipelineLayoutCache;
class ProgramResourceVk;
class ShaderModuleCache;

/*!
 * \ingroup Vulkan
 */
class ProgramVk
:	public IProgram
,	public Context::ICleanupListener
{
	T_RTTI_CLASS;

public:
	class DescriptorSetKey : public StaticVector< intptr_t, 32 >
	{
	public:
		bool operator < (const DescriptorSetKey& rh) const;

		bool operator > (const DescriptorSetKey& rh) const;
	};

	explicit ProgramVk(Context* context, uint32_t& instances);

	virtual ~ProgramVk();

	bool create(ShaderModuleCache* shaderModuleCache, PipelineLayoutCache* pipelineLayoutCache, const ProgramResourceVk* resource, int32_t maxAnistropy, float mipBias, const wchar_t* const tag);

	bool validateGraphics(CommandBuffer* commandBuffer, const float targetSize[2]);

	bool validateCompute(CommandBuffer* commandBuffer);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setImageViewParameter(handle_t handle, ITexture* imageView) override final;

	virtual void setBufferViewParameter(handle_t handle, const IBufferView* bufferView) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	const RenderState& getRenderState() const { return m_renderState; }

	VkShaderModule getVertexVkShaderModule() const { return m_vertexShaderModule; }

	VkShaderModule getFragmentVkShaderModule() const { return m_fragmentShaderModule; }

	VkShaderModule getComputeVkShaderModule() const { return m_computeShaderModule; }

	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

	uint32_t getShaderHash() const { return m_shaderHash; }

#if defined(_DEBUG)
	const std::wstring& getTag() const { return m_tag; }
#endif

private:
	struct ParameterMap
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		uint32_t buffer = 0;	//!< Uniform buffer index.
		uint32_t offset = 0;	//!< Offset into uniform buffer's data.
		uint32_t size = 0;		//!< Number of floats.
	};

	struct UniformBuffer
	{
		uint32_t size = 0;
		uint32_t alignedSize = 0;
		AlignedVector< float > data;
		UniformBufferRange range;
		bool dirty = true;
	};

	struct Sampler
	{
		uint32_t binding;
		VkSampler sampler;
	};

//	struct Texture
//	{
//#if defined(_DEBUG)
//		std::wstring name;
//#endif
//		uint32_t binding;
//		Ref< ITexture > texture;
//	};

	struct Image
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		uint32_t binding;
		Ref< ITexture > texture;
	};

	struct SBuffer
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		uint32_t binding;
		const BufferViewVk* bufferView = nullptr;
	};

	std::wstring m_tag;
	Context* m_context = nullptr;
	uint32_t& m_instances;
	RenderState m_renderState;
	VkShaderModule m_vertexShaderModule = 0;
	VkShaderModule m_fragmentShaderModule = 0;
	VkShaderModule m_computeShaderModule = 0;
	VkDescriptorSetLayout m_descriptorSetLayout = 0;
	VkPipelineLayout m_pipelineLayout = 0;
	uint32_t m_descriptorPoolRevision = ~0U;
	SmallMap< DescriptorSetKey, VkDescriptorSet > m_descriptorSets;
	VkDescriptorSet m_descriptorSet = 0;
	UniformBuffer m_uniformBuffers[3];
	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< Sampler > m_samplers;
	//AlignedVector< Texture > m_textures;
	AlignedVector< Image > m_images;
	AlignedVector< SBuffer > m_sbuffers;
	uint32_t m_stencilReference = 0;
	uint32_t m_shaderHash = 0;

	bool validateDescriptorSet();

	virtual void postCleanup() override final;
};

}
