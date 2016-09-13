#ifndef traktor_render_SimpleTextureCapture_H
#define traktor_render_SimpleTextureCapture_H

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class SimpleTextureCapture : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureCapture(ISimpleTexture* texture);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< ISimpleTexture > m_texture;
	int32_t m_locked;
};
		
	}
}

#endif	// traktor_render_SimpleTextureCapture_H
