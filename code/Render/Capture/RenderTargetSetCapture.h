#pragma once

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

	virtual void destroy() override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void swap(int index1, int index2) override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int index, void* buffer) const override final;

	RenderTargetSet* getRenderTargetSet() const { return m_renderTargetSet; }

	bool haveColorTexture(int index) const;

	bool haveDepthTexture() const;

private:
	Ref< RenderTargetSet > m_renderTargetSet;
	mutable Ref< SimpleTextureCapture > m_colorTextures[4];
	mutable Ref< SimpleTextureCapture > m_depthTexture;
};

	}
}

