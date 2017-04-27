/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VolumeTexturePs4_H
#define traktor_render_VolumeTexturePs4_H

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{
		
class ContextPs4;
struct VolumeTextureCreateDesc;

/*!
 * \ingroup GNM
 */
class VolumeTexturePs4 : public IVolumeTexture
{
	T_RTTI_CLASS;
	
public:
	VolumeTexturePs4(ContextPs4* context);

	virtual ~VolumeTexturePs4();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;
};
		
	}
}

#endif	// traktor_render_VolumeTexturePs4_H
