#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticVector.h"
#include "Render/IProgram.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class PipelineLayoutCache;
class ProgramResourceVk;
class ShaderModuleCache;
class UniformBufferPoolVk;

/*!
 * \ingroup Vulkan
 */
class ProgramVk : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator
	);

	virtual ~ProgramVk();

	bool create(ShaderModuleCache* shaderModuleCache, PipelineLayoutCache* pipelineLayoutCache, const ProgramResourceVk* resource, int32_t maxAnistropy, float mipBias, const wchar_t* const tag);

	bool validateGraphics(VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool, float targetSize[2]);

	bool validateCompute(VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setStructBufferParameter(handle_t handle, StructBuffer* structBuffer) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	const RenderState& getRenderState() const { return m_renderState; }

	VkShaderModule getVertexVkShaderModule() const { return m_vertexShaderModule; }

	VkShaderModule getFragmentVkShaderModule() const { return m_fragmentShaderModule; }

	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

	uint32_t getShaderHash() const { return m_shaderHash; }

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
		AlignedVector< float > data;
		VkBuffer buffer = 0;
		VmaAllocation allocation = 0;
		void* ptr = nullptr;
		bool dirty = true;
	};

	struct Sampler
	{
		uint32_t binding;
		VkSampler sampler;
	};

	struct Texture
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
		Ref< StructBuffer > sbuffer;
	};

#if defined(_DEBUG)
	std::wstring m_tag;
#endif

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	RenderState m_renderState;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
	VkShaderModule m_computeShaderModule;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;
	UniformBuffer m_uniformBuffers[3];
	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< Sampler > m_samplers;
	AlignedVector< Texture > m_textures;
	AlignedVector< SBuffer > m_sbuffers;
	StaticVector< VkDescriptorBufferInfo, 16 > m_bufferInfos;
	StaticVector< VkDescriptorImageInfo, 16 > m_imageInfos;
	StaticVector< VkWriteDescriptorSet, 16 + 16 > m_writes;
	uint32_t m_stencilReference;
	uint32_t m_shaderHash;
};

	}
}

