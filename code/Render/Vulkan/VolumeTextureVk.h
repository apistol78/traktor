/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTextureVk_H
#define traktor_render_VolumeTextureVk_H

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{
		
struct VolumeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class VolumeTextureVk : public IVolumeTexture
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureVk();

	virtual ~VolumeTextureVk();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;
};
		
	}
}

#endif	// traktor_render_VolumeTextureVk_H
