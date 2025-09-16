/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Render/Frame/RenderGraphTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;
class RenderGraphTargetSetPool;
class RenderGraphBufferPool;
class RenderGraphTexturePool;

/*!
 * \ingroup Render
 */
class T_DLLCLASS RenderGraphContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit RenderGraphContext(IRenderSystem* renderSystem);

	virtual ~RenderGraphContext();

	void destroy();

	void cleanup();

	RenderGraphTargetSetPool* getTargetSetPool() const { return m_targetSetPool; }

	RenderGraphBufferPool* getBufferPool() const { return m_bufferPool; }

	RenderGraphTexturePool* getTexturePool() const { return m_texturePool; }

private:
	Ref< RenderGraphTargetSetPool > m_targetSetPool;
	Ref< RenderGraphBufferPool > m_bufferPool;
	Ref< RenderGraphTexturePool > m_texturePool;
};

}
