/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Frame/RenderGraphContext.h"
#include "Render/Frame/RenderGraphBufferPool.h"
#include "Render/Frame/RenderGraphTargetSetPool.h"
#include "Render/Frame/RenderGraphTexturePool.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphContext", RenderGraphContext, Object)

RenderGraphContext::RenderGraphContext(IRenderSystem* renderSystem)
	: m_targetSetPool(new RenderGraphTargetSetPool(renderSystem))
	, m_bufferPool(new RenderGraphBufferPool(renderSystem))
	, m_texturePool(new RenderGraphTexturePool(renderSystem))
{
}

RenderGraphContext::~RenderGraphContext()
{
	T_FATAL_ASSERT_M(m_targetSetPool == nullptr, L"Forgot to destroy RenderGraphContext instance.");
	T_FATAL_ASSERT_M(m_bufferPool == nullptr, L"Forgot to destroy RenderGraphContext instance.");
	T_FATAL_ASSERT_M(m_texturePool == nullptr, L"Forgot to destroy RenderGraphContext instance.");
}

void RenderGraphContext::destroy()
{
	safeDestroy(m_targetSetPool);
	safeDestroy(m_bufferPool);
	safeDestroy(m_texturePool);
}

void RenderGraphContext::cleanup()
{
	m_targetSetPool->cleanup();
	m_texturePool->cleanup();
}

}
