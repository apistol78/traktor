/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/IRenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
class RenderTargetDepthDx11;
class RenderTargetDx11;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup DX11
 */
class RenderTargetSetDx11 : public IRenderTargetSet
{
	T_RTTI_CLASS;

public:
	explicit RenderTargetSetDx11(ContextDx11* context);

	virtual ~RenderTargetSetDx11();

	bool create(const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* sharedDepthStencil);

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ITexture* getColorTexture(int32_t index) const override final;

	virtual ITexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	void setContentValid(bool contentValid) {
		m_contentValid = contentValid;
	}

	bool usingPrimaryDepthStencil() const {
		return m_usingPrimaryDepthStencil;
	}

private:
	Ref< ContextDx11 > m_context;
	RefArray< RenderTargetDx11 > m_colorTextures;
	Ref< RenderTargetDepthDx11 > m_depthTexture;
	int32_t m_width;
	int32_t m_height;
	bool m_contentValid;
	bool m_sharedDepthStencil;
	bool m_usingPrimaryDepthStencil;
};

	}
}

