/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeTextureDx11_H
#define traktor_render_CubeTextureDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ICubeTexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct CubeTextureCreateDesc;

/*!
 * \ingroup DX11
 */
class CubeTextureDx11 : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureDx11(ContextDx11* context);

	virtual ~CubeTextureDx11();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual int getDepth() const override final;

	virtual bool lock(int side, int level, Lock& lock) override final;

	virtual void unlock(int side, int level) override final;

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11Texture2D > m_d3dTextureStaging;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int m_side;
	int m_mipCount;
};
		
	}
}

#endif	// traktor_render_CubeTextureDx11_H
