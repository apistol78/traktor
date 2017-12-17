/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetSw_H
#define traktor_render_RenderTargetSetSw_H

#include <vector>
#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;
class RenderTargetSw;

/*!
 * \ingroup SW
 */
class RenderTargetSetSw : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetSw();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual ISimpleTexture* getDepthTexture() const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool isContentValid() const;

	virtual bool read(int index, void* buffer) const;

	float* getDepthSurface() { return m_depthSurface.ptr(); }

	uint8_t* getStencilSurface() { return m_stencilSurface.ptr(); }

	bool usingPrimaryDepth() const { return m_usingPrimaryDepth; }

private:
	RefArray< RenderTargetSw > m_colorTargets;
	AutoArrayPtr< float, AllocFreeAlign > m_depthSurface;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_stencilSurface;
	int m_width;
	int m_height;
	bool m_usingPrimaryDepth;
};

	}
}

#endif	// traktor_render_RenderTargetSetSw_H
