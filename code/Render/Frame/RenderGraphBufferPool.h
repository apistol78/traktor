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

namespace traktor::render
{

class Buffer;
class IRenderSystem;

/*!
 * \ingroup Render
 */
class RenderGraphBufferPool : public Object
{
	T_RTTI_CLASS;

public:
	explicit RenderGraphBufferPool(IRenderSystem* renderSystem);

	void destroy();

	Ref< Buffer > acquire(uint32_t bufferSize, uint32_t persistentHandle);

	void release(Ref< Buffer >& buffer);

private:
	struct Pool
	{
		// Pool identification.
		uint32_t bufferSize;
		uint32_t persistentHandle;

		// Pool buffers.
		RefArray< Buffer > free;
		RefArray< Buffer > acquired;
	};

	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

}
