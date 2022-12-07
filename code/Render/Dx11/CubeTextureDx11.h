/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/ComRef.h"
#include "Render/ITexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct CubeTextureCreateDesc;

/*!
 * \ingroup DX11
 */
class CubeTextureDx11 : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit CubeTextureDx11(ContextDx11* context);

	virtual ~CubeTextureDx11();

	bool create(const CubeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

	ID3D11Texture2D* getD3D11Texture2D() const {
		return m_d3dTexture;
	}

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const {
		return m_d3dTextureResourceView;
	}

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11Texture2D > m_d3dTextureStaging;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int32_t m_side;
	int32_t m_mipCount;
};

	}
}
