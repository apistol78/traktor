#ifndef traktor_render_RenderTargetDx11_H
#define traktor_render_RenderTargetDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

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
class RenderTargetDx11 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDx11(ContextDx11* context);

	virtual ~RenderTargetDx11();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	void unbind();

	ID3D11RenderTargetView* getD3D11RenderTargetView() const {
		return m_d3dRenderTargetView;
	}

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const {
		return m_d3dTextureResourceView;
	}

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11Texture2D > m_d3dTextureRead;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	DXGI_FORMAT m_d3dColorFormat;
	int32_t m_width;
	int32_t m_height;
	bool m_generateMips;
};

	}
}

#endif	// traktor_render_RenderTargetDx11_H
