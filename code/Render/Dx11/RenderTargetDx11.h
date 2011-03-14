#ifndef traktor_render_RenderTargetDx11_H
#define traktor_render_RenderTargetDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ITexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS RenderTargetDx11 : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDx11(ContextDx11* context);

	virtual ~RenderTargetDx11();

	bool create(ID3D11Device* d3dDevice, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	inline ID3D11RenderTargetView* getD3D11RenderTargetView() const {
		return m_d3dRenderTargetView;
	}

	inline ID3D11ShaderResourceView* getD3D11TextureResourceView() const {
		return m_d3dTextureResourceView;
	}

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDx11_H
