/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/IRenderTargetSet.h"
#include "Render/Types.h"

namespace traktor::render
{

class ResourceTracker;
class TextureVrfy;

/*!
 * \ingroup Vrfy
 */
class RenderTargetSetVrfy : public IRenderTargetSet
{
	T_RTTI_CLASS;

public:
	explicit RenderTargetSetVrfy(ResourceTracker* resourceTracker, const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* renderTargetSet);

	virtual ~RenderTargetSetVrfy();

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ITexture* getColorTexture(int32_t index) const override final;

	virtual ITexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	IRenderTargetSet* getRenderTargetSet() const { return m_renderTargetSet; }

	bool haveColorTexture(int32_t index) const;

	bool haveDepthTexture() const;

	bool usingPrimaryDepthStencil() const;

	uint32_t getMultiSample() const;

private:
	Ref< ResourceTracker > m_resourceTracker;
	RenderTargetSetCreateDesc m_setDesc;
	Ref< IRenderTargetSet > m_renderTargetSet;
	mutable Ref< TextureVrfy > m_colorTextures[4];
	mutable Ref< TextureVrfy > m_depthTexture;
};

}
