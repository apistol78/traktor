/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTextureDx11_H
#define traktor_render_SimpleTextureDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup DX11
 */
class SimpleTextureDx11 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx11(ContextDx11* context);

	virtual ~SimpleTextureDx11();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int m_width;
	int m_height;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx11_H
