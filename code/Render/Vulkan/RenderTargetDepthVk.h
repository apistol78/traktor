#ifndef traktor_render_RenderTargetDepthVk_H
#define traktor_render_RenderTargetDepthVk_H

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetDepthVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthVk();

	virtual ~RenderTargetDepthVk();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

private:
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthVk_H
