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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;
};
		
	}
}

#endif	// traktor_render_VolumeTextureVk_H
