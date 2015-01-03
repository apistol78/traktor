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

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

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
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int32_t m_width;
	int32_t m_height;
	bool m_generateMips;
};

	}
}

#endif	// traktor_render_RenderTargetDx11_H
