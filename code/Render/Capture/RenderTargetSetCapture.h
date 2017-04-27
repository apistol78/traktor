/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetCapture_H
#define traktor_render_RenderTargetSetCapture_H

#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class SimpleTextureCapture;

/*!
 * \ingroup Capture
 */
class RenderTargetSetCapture : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetCapture(RenderTargetSet* renderTargetSet);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getColorTexture(int index) const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getDepthTexture() const T_OVERRIDE T_FINAL;

	virtual void swap(int index1, int index2) T_OVERRIDE T_FINAL;

	virtual void discard() T_OVERRIDE T_FINAL;

	virtual bool read(int index, void* buffer) const T_OVERRIDE T_FINAL;

	RenderTargetSet* getRenderTargetSet() const { return m_renderTargetSet; }

private:
	Ref< RenderTargetSet > m_renderTargetSet;
	mutable Ref< SimpleTextureCapture > m_colorTextures[4];
	mutable Ref< SimpleTextureCapture > m_depthTexture;
};

	}
}

#endif	// traktor_render_RenderTargetSetCapture_H
