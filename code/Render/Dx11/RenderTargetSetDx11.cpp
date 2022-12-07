/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/RenderTargetDepthDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetDx11", RenderTargetSetDx11, IRenderTargetSet)

RenderTargetSetDx11::RenderTargetSetDx11(ContextDx11* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_contentValid(false)
,	m_sharedDepthStencil(false)
,	m_usingPrimaryDepthStencil(false)
{
}

RenderTargetSetDx11::~RenderTargetSetDx11()
{
	destroy();
}

bool RenderTargetSetDx11::create(const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* sharedDepthStencil)
{
	m_colorTextures.resize(setDesc.count);
	for (int32_t i = 0; i < setDesc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetDx11(m_context);
		if (!m_colorTextures[i]->create(setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTexture = new RenderTargetDepthDx11(m_context);
		if (!m_depthTexture->create(setDesc))
			return false;
	}
	else if (sharedDepthStencil)
	{
		m_depthTexture = dynamic_type_cast< RenderTargetDepthDx11* >(sharedDepthStencil->getDepthTexture());
		m_sharedDepthStencil = true;
	}
	else
	{
		m_usingPrimaryDepthStencil = setDesc.usingPrimaryDepthStencil;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetSetDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	// Destroy color target textures.
	for (auto colorTexture : m_colorTextures)
	{
		if (colorTexture)
			colorTexture->destroy();
	}
	m_colorTextures.clear();

	// Only destroy depth target texture if not being shared, else just release reference.
	if (!m_sharedDepthStencil)
		safeDestroy(m_depthTexture);
	else
		m_depthTexture = nullptr;
}

int32_t RenderTargetSetDx11::getWidth() const
{
	return m_width;
}

int32_t RenderTargetSetDx11::getHeight() const
{
	return m_height;
}

ITexture* RenderTargetSetDx11::getColorTexture(int32_t index) const
{
	return index < (int32_t)m_colorTextures.size() ? m_colorTextures[index] : 0;
}

ITexture* RenderTargetSetDx11::getDepthTexture() const
{
	return m_depthTexture;
}

bool RenderTargetSetDx11::isContentValid() const
{
	return m_contentValid;
}

bool RenderTargetSetDx11::read(int32_t index, void* buffer) const
{
	return m_colorTextures[index]->read(buffer);
}

	}
}
