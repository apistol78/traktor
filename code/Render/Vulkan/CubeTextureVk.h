/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	virtual bool lock(int side, int level, Lock& lock) override final;

	virtual void unlock(int side, int level) override final;
};
		
	}
}

#endif	// traktor_render_CubeTextureVk_H
