#ifndef traktor_render_CubeTextureVk_H
#define traktor_render_CubeTextureVk_H

#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

struct CubeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class CubeTextureVk : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureVk();

	virtual ~CubeTextureVk();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual bool lock(int side, int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int side, int level) T_OVERRIDE T_FINAL;
};
		
	}
}

#endif	// traktor_render_CubeTextureVk_H
