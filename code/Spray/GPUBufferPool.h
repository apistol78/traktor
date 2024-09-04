/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IRenderSystem;

}

namespace traktor::spray
{

class T_DLLCLASS GPUBufferPool : public Object
{
	T_RTTI_CLASS

public:
	explicit GPUBufferPool(render::IRenderSystem* renderSystem);

	void destroy();

	bool allocBuffers(uint32_t capacity, Ref< render::Buffer >& outHeadBuffer, Ref< render::Buffer >& outPointBuffer);

	void freeBuffers(Ref< render::Buffer >& inoutHeadBuffer, Ref< render::Buffer >& inoutPointBuffer);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::Buffer > m_headBuffers;
	RefArray< render::Buffer > m_pointBuffers;
	bool m_destroyed;
};

}
