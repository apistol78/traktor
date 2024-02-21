/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
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

namespace traktor::render
{

class IRenderSystem;
class ITexture;

/*!
 * \ingroup Render
 */
class RenderGraphTexturePool : public Object
{
	T_RTTI_CLASS;

public:
	explicit RenderGraphTexturePool(IRenderSystem* renderSystem);

	void destroy();

	Ref< ITexture > acquire(const RenderGraphTextureDesc& textureDesc, uint32_t persistentHandle);

	void release(Ref< ITexture >& texture);

private:
	struct Pool
	{
		// Pool identification.
		RenderGraphTextureDesc textureDesc;
		uint32_t persistentHandle;

		// Pool buffers.
		RefArray< ITexture > free;
		RefArray< ITexture > acquired;
	};

	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

}
