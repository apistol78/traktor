/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Render/VertexElement.h"

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class IVertexLayout;

}

namespace traktor::spark
{

class AccShapeVertexPool : public Object
{
	T_RTTI_CLASS;

public:
	explicit AccShapeVertexPool(render::IRenderSystem* renderSystem);

	bool create(const AlignedVector< render::VertexElement >& vertexElements);

	void destroy();

	bool acquire(int32_t vertexCount, Ref< render::Buffer >& outVertexBuffer);

	void release(render::Buffer* vertexBuffer);

	const render::IVertexLayout* getVertexLayout() const { return m_vertexLayout; }

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< const render::IVertexLayout > m_vertexLayout;
	RefArray< render::Buffer > m_freeBuffers;
	uint32_t m_vertexSize = 0;
};

}
