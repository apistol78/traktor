/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureCapture_H
#define traktor_render_CubeTextureCapture_H

#include "Core/Ref.h"
#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class CubeTextureCapture : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureCapture(ICubeTexture* texture);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	virtual bool lock(int side, int level, Lock& lock) override final;

	virtual void unlock(int side, int level) override final;

	ICubeTexture* getTexture() const { return m_texture; }

private:
	Ref< ICubeTexture > m_texture;
	int32_t m_locked[2];
};
		
	}
}

#endif	// traktor_render_CubeTextureCapture_H
