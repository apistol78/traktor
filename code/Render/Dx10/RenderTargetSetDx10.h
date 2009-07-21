#ifndef traktor_render_RenderTargetSetDx10_H
#define traktor_render_RenderTargetSetDx10_H

#include "Render/RenderTargetSet.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS RenderTargetSetDx10 : public RenderTargetSet
{
	T_RTTI_CLASS(RenderTargetSetDx10)

public:
	RenderTargetSetDx10(ContextDx10* context);

	virtual ~RenderTargetSetDx10();

	bool create(ID3D10Device* d3dDevice, const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ITexture* getColorTexture(int index) const;

	inline ID3D10DepthStencilView* getD3D10DepthTextureView() const {
		return m_d3dDepthTextureView;
	}

private:
	Ref< ContextDx10 > m_context;
	RefArray< RenderTargetDx10 > m_colorTextures;
	ComRef< ID3D10Texture2D > m_d3dDepthTexture;
	ComRef< ID3D10DepthStencilView > m_d3dDepthTextureView;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetSetDx10_H
