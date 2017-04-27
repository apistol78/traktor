/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureCapture_H
#define traktor_render_CubeTextureCapture_H

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual bool lock(int side, int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int side, int level) T_OVERRIDE T_FINAL;

	ICubeTexture* getTexture() const { return m_texture; }

private:
	Ref< ICubeTexture > m_texture;
	int32_t m_locked[2];
};
		
	}
}

#endif	// traktor_render_CubeTextureCapture_H
