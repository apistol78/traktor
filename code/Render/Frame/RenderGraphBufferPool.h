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

	Ref< Buffer > acquire(uint32_t elementCount, uint32_t elementSize);

	void release(Ref< Buffer >& buffer);

private:
	Ref< IRenderSystem > m_renderSystem;
	SmallMap< std::pair< uint32_t, uint32_t >, RefArray< Buffer > > m_pool;
};

}
