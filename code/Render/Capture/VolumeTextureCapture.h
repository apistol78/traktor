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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	IVolumeTexture* getTexture() const { return m_texture; }

private:
	Ref< IVolumeTexture > m_texture;
};
		
	}
}

#endif	// traktor_render_VolumeTextureCapture_H
