#ifndef traktor_render_RenderTargetSetDx11_H
#define traktor_render_RenderTargetSetDx11_H

#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
class RenderTargetDepthDx11;
class RenderTargetDx11;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup DX11
 */
class RenderTargetSetDx11 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetDx11(ContextDx11* context);

	virtual ~RenderTargetSetDx11();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual ISimpleTexture* getDepthTexture() const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool read(int index, void* buffer) const;

	bool usingPrimaryDepthStencil() const {
		return m_usingPrimaryDepthStencil;
	}

private:
	Ref< ContextDx11 > m_context;
	RefArray< RenderTargetDx11 > m_colorTextures;
	Ref< RenderTargetDepthDx11 > m_depthTexture;
	int m_width;
	int m_height;
	bool m_usingPrimaryDepthStencil;
};

	}
}

#endif	// traktor_render_RenderTargetSetDx11_H
