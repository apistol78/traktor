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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual int getDepth() const T_OVERRIDE T_FINAL;

	virtual bool lock(int side, int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int side, int level) T_OVERRIDE T_FINAL;

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int m_side;
	int m_mipCount;
};
		
	}
}

#endif	// traktor_render_CubeTextureDx11_H
