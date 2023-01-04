/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Plane.h"

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
class IVertexLayout;
class RenderContext;
class Shader;

}

namespace traktor::world
{

class IWorldRenderPass;

}

namespace traktor::spray
{

struct TrailVertex;

/*! Ribbon trail renderer.
 * \ingroup Spray
 */
class T_DLLCLASS TrailRenderer : public Object
{
	T_RTTI_CLASS;

public:
	explicit TrailRenderer(render::IRenderSystem* renderSystem);

	virtual ~TrailRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const CircularVector< Vector4, 64 >& points,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		float width,
		float time,
		float age
	);

	void flush(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	struct Batch
	{
		render::Shader* shader;
		uint32_t points;
		Vector4 timeAndAge;
	};

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffers[4];
	Ref< render::Buffer > m_indexBuffer;
	uint32_t m_count;
	TrailVertex* m_vertex;
	AlignedVector< Batch > m_batches;
};

}
