#pragma once

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

	virtual void destroy() override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int index, void* buffer) const override final;

	RenderTargetPs4* getColorTargetPs4(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthPs4* getDepthTargetPs4() const { return m_depthTarget; }

private:
	Ref< ContextPs4 > m_context;
	RefArray< RenderTargetPs4 > m_colorTargets;
	Ref< RenderTargetDepthPs4 > m_depthTarget;
};

	}
}

