/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTextureCapture_H
#define traktor_render_VolumeTextureCapture_H

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class VolumeTextureCapture : public IVolumeTexture
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureCapture(IVolumeTexture* texture);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	IVolumeTexture* getTexture() const { return m_texture; }

private:
	Ref< IVolumeTexture > m_texture;
};
		
	}
}

#endif	// traktor_render_VolumeTextureCapture_H
