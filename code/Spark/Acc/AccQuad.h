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
#include "Core/Math/Aabb2.h"
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

class ColorTransform;

/*! Simple unit-quad shape.
 * \ingroup Spark
 */
class AccQuad : public Object
{
public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	void destroy();

	void render(
		render::RenderPass* renderPass,
		const Aabb2& bounds,
		const Matrix33& transform,
		const Vector4& frameSize,
		const Vector4& frameTransform,
		const ColorTransform& cxform,
		render::ITexture* texture,
		const Vector4& textureOffset,
		bool maskWrite,
		bool maskIncrement,
		uint8_t maskReference
	) const;

	void blit(
		render::RenderPass* renderPass,
		render::ITexture* texture
	) const;

private:
	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;
	resource::Proxy< render::Shader > m_shaderBlit;
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
};

}
