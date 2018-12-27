/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< ISimpleTexture > m_texture;
	int32_t m_locked;
};
		
	}
}

#endif	// traktor_render_SimpleTextureCapture_H
