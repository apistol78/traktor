/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetPs4_H
#define traktor_render_RenderTargetSetPs4_H

#include "Core/RefArray.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
class RenderTargetDepthPs4;
class RenderTargetPs4;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup GNM
 */
class RenderTargetSetPs4 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetPs4(ContextPs4* context);

	virtual ~RenderTargetSetPs4();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getColorTexture(int index) const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getDepthTexture() const T_OVERRIDE T_FINAL;

	virtual void swap(int index1, int index2) T_OVERRIDE T_FINAL;

	virtual void discard() T_OVERRIDE T_FINAL;

	virtual bool isContentValid() const T_OVERRIDE T_FINAL;

	virtual bool read(int index, void* buffer) const T_OVERRIDE T_FINAL;

	RenderTargetPs4* getColorTargetPs4(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthPs4* getDepthTargetPs4() const { return m_depthTarget; }

private:
	Ref< ContextPs4 > m_context;
	RefArray< RenderTargetPs4 > m_colorTargets;
	Ref< RenderTargetDepthPs4 > m_depthTarget;
};

	}
}

#endif	// traktor_render_RenderTargetSetPs4_H
