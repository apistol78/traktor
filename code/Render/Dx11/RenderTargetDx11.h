#pragma once

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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	void unbind();

	bool read(void* buffer) const;

	ID3D11Texture2D* getD3D11Texture2D() const {
		return m_d3dTexture;
	}

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
	ComRef< ID3D11Texture2D > m_d3dTextureStaging;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	DXGI_FORMAT m_d3dColorFormat;
	int32_t m_width;
	int32_t m_height;
	bool m_generateMips;
};

	}
}
