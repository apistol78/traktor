#ifndef traktor_render_ProgramVk_H
#define traktor_render_ProgramVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

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
	ProgramVk(VkDevice device);

	virtual ~ProgramVk();

	bool create(const ProgramResourceVk* resource);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setFloatParameter(handle_t handle, float param) T_OVERRIDE T_FINAL;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) T_OVERRIDE T_FINAL;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) T_OVERRIDE T_FINAL;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) T_OVERRIDE T_FINAL;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) T_OVERRIDE T_FINAL;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) T_OVERRIDE T_FINAL;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) T_OVERRIDE T_FINAL;

	virtual void setStencilReference(uint32_t stencilReference) T_OVERRIDE T_FINAL;

private:
	VkDevice m_device;
};

	}
}

#endif	// traktor_render_ProgramVk_H
