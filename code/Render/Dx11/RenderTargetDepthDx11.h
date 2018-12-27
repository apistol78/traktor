/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetDepthDx11_H
#define traktor_render_RenderTargetDepthDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup DX11
 */
class RenderTargetDepthDx11 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthDx11(ContextDx11* context);

	virtual ~RenderTargetDepthDx11();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;

	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	ID3D11DepthStencilView* getD3D11DepthTextureView() const {
		return m_d3dDepthTextureView;
	}

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const {
		return m_d3dTextureResourceView;
	}

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11DepthStencilView > m_d3dDepthTextureView;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthDx11_H
