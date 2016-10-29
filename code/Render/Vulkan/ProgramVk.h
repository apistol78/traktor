#ifndef traktor_render_ProgramVk_H
#define traktor_render_ProgramVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ProgramResourceVk;

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

	bool validate(VkDevice device, VkDescriptorSet descriptorSet);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setFloatParameter(handle_t handle, float param) T_OVERRIDE T_FINAL;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) T_OVERRIDE T_FINAL;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) T_OVERRIDE T_FINAL;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) T_OVERRIDE T_FINAL;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) T_OVERRIDE T_FINAL;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) T_OVERRIDE T_FINAL;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) T_OVERRIDE T_FINAL;

	virtual void setStencilReference(uint32_t stencilReference) T_OVERRIDE T_FINAL;

	VkShaderModule getVertexVkShaderModule() const { return m_vertexShaderModule; }

	VkShaderModule getFragmentVkShaderModule() const { return m_fragmentShaderModule; }

private:
	struct ParameterMap
	{
		uint32_t offset;	//!< Offset into m_parameterScalarData
		uint32_t size;		//!< Number of floats.
		uint32_t uniformBufferOffset;

		ParameterMap()
		:	offset(0)
		,	size(0)
		,	uniformBufferOffset(0)
		{
		}
	};

	struct UniformBuffer
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		uint32_t size;

		AlignedVector< ParameterMap > parameters;

		UniformBuffer()
		:	buffer(0)
		,	memory(0)
		,	size(0)
		{
		}
	};

	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
	
	UniformBuffer m_vertexUniformBuffers[3];
	UniformBuffer m_fragmentUniformBuffers[3];

	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< float > m_parameterScalarData;
};

	}
}

#endif	// traktor_render_ProgramVk_H
