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
#include "Core/Math/Matrix33.h"
#include "Resource/Proxy.h"

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ITexture;
class IVertexLayout;
class RenderPass;
class Shader;

}

namespace traktor::spark
{

class AccGlyph : public Object
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	void beginFrame();

	void endFrame();

	void add(
		const Aabb2& bounds,
		const Matrix33& transform,
		const Vector4& textureOffset
	);

	void render(
		render::RenderPass* renderPass,
		render::handle_t glyphCacheTargetSetId,
		const Vector4& frameSize,
		const Vector4& frameTransform,
		uint8_t maskReference,
		uint8_t glyphFilter,
		const Color4f& glyphColor,
		const Color4f& glyphFilterColor
	);

private:
	resource::Proxy< render::Shader > m_shaderGlyph;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	uint8_t* m_vertex = nullptr;
	uint32_t m_offset = 0;
	uint32_t m_count = 0;
};

}
