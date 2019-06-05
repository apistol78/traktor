#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#endif

#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ProgramResourceVk;
class UniformBufferPoolVk;

/*!
 * \ingroup Vulkan
 */
class ProgramVk : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramVk();

	virtual ~ProgramVk();

	bool create(VkPhysicalDevice physicalDevice, VkDevice device, const ProgramResourceVk* resource);

	bool validateGraphics(VkDevice device, VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool, float targetSize[2]);

	bool validateCompute(VkDevice device, VkDescriptorPool descriptorPool, VkCommandBuffer commandBuffer, UniformBufferPoolVk* uniformBufferPool);

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

	uint32_t getHash() const { return m_hash; }

private:
	struct ParameterMap
	{
		uint32_t buffer;	//!< Uniform buffer index.
		uint32_t offset;	//!< Offset into uniform buffer's data.
		uint32_t size;		//!< Number of floats.

		ParameterMap()
		:	buffer(0)
		,	offset(0)
		,	size(0)
		{
		}
	};

	struct UniformBuffer
	{
		uint32_t size;
		AlignedVector< float > data;
		VkBuffer buffer;
		VkDeviceMemory memory;
		bool dirty;

		UniformBuffer()
		:	size(0)
		,	buffer(0)
		,	memory(0)
		,	dirty(true)
		{
		}
	};

	struct Sampler
	{
		uint32_t binding;
		VkSampler sampler;
	};

	struct Texture
	{
		uint32_t binding;
		Ref< ITexture > texture;
	};

	struct SBuffer
	{
		uint32_t binding;
		Ref< StructBuffer > sbuffer;
	};

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
	uint32_t m_hash;
};

	}
}

