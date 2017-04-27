/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSw_H
#define traktor_render_RenderTargetSw_H

#include "Core/Misc/AutoPtr.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class AbstractSampler;

struct RenderTargetCreateDesc;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup SW
 */
class RenderTargetSw : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetSw();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	uint32_t* getColorSurface();

	Ref< AbstractSampler > createSampler() const;

private:
	AutoArrayPtr< uint32_t > m_color;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetSw_H
