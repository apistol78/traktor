/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/ResourceTracker.h"
#include "Render/Vrfy/TextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVrfy", RenderTargetSetVrfy, IRenderTargetSet)

RenderTargetSetVrfy::RenderTargetSetVrfy(ResourceTracker* resourceTracker, const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* renderTargetSet)
:	m_resourceTracker(resourceTracker)
,	m_setDesc(setDesc)
,	m_renderTargetSet(renderTargetSet)
{
	m_resourceTracker->add(this);
}

RenderTargetSetVrfy::~RenderTargetSetVrfy()
{
	m_resourceTracker->remove(this);
}

void RenderTargetSetVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set already destroyed.");

	m_colorTextures[0].reset();
	m_colorTextures[1].reset();
	m_colorTextures[2].reset();
	m_colorTextures[3].reset();
	m_depthTexture.reset();

	safeDestroy(m_renderTargetSet);
}

int32_t RenderTargetSetVrfy::getWidth() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getWidth() : 0;
}

int32_t RenderTargetSetVrfy::getHeight() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getHeight() : 0;
}

ITexture* RenderTargetSetVrfy::getColorTexture(int32_t index) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return nullptr;

	if (!m_colorTextures[index])
	{
		Ref< ITexture > colorTexture = m_renderTargetSet->getColorTexture(index);
		if (!colorTexture)
			return nullptr;

		m_colorTextures[index] = new TextureVrfy(m_resourceTracker, colorTexture);
	}

	return m_colorTextures[index];
}

ITexture* RenderTargetSetVrfy::getDepthTexture() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return nullptr;

	if (!m_depthTexture)
	{
		Ref< ITexture > depthTexture = m_renderTargetSet->getDepthTexture();
		if (!depthTexture)
			return nullptr;

		m_depthTexture = new TextureVrfy(m_resourceTracker, depthTexture);
	}

	return m_depthTexture;
}

bool RenderTargetSetVrfy::isContentValid() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->isContentValid() : false;
}

bool RenderTargetSetVrfy::read(int32_t index, void* buffer) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	T_CAPTURE_ASSERT (index >= 0, L"Incorrect read-back index.");
	return m_renderTargetSet ? m_renderTargetSet->read(index, buffer) : false;
}

bool RenderTargetSetVrfy::haveColorTexture(int32_t index) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getColorTexture(index) != nullptr : false;
}

bool RenderTargetSetVrfy::haveDepthTexture() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getDepthTexture() != nullptr : false;
}

bool RenderTargetSetVrfy::usingPrimaryDepthStencil() const
{
	return m_setDesc.usingPrimaryDepthStencil;
}

uint32_t RenderTargetSetVrfy::getMultiSample() const
{
	return m_setDesc.multiSample;
}

	}
}
