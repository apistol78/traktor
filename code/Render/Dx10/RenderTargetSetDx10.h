#ifndef traktor_render_RenderTargetSetDx10_H
#define traktor_render_RenderTargetSetDx10_H

#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class ContextDx10;
class RenderTargetDx10;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup DX10
 */
class RenderTargetSetDx10 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetDx10(ContextDx10* context);

	virtual ~RenderTargetSetDx10();

	bool create(ID3D10Device* d3dDevice, const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool read(int index, void* buffer) const;

	inline ID3D10DepthStencilView* getD3D10DepthTextureView() const {
		return m_d3dDepthTextureView;
	}

	inline bool usingPrimaryDepthStencil() const {
		return m_usingPrimaryDepthStencil;
	}

private:
	Ref< ContextDx10 > m_context;
	RefArray< RenderTargetDx10 > m_colorTextures;
	ComRef< ID3D10Texture2D > m_d3dDepthTexture;
	ComRef< ID3D10DepthStencilView > m_d3dDepthTextureView;
	int m_width;
	int m_height;
	bool m_usingPrimaryDepthStencil;
};

	}
}

#endif	// traktor_render_RenderTargetSetDx10_H
