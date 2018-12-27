/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSw_H
#define traktor_render_RenderTargetSw_H

#include "Core/Ref.h"
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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;

	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

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
