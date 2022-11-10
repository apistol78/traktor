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
#include "Core/Math/Plane.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector2.h"
#include "Core/Containers/AlignedVector.h"
#include "Spray/Point.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Buffer;
class IRenderSystem;
class IVertexLayout;
class RenderContext;

	}

	namespace world
	{

class IWorldRenderPass;

	}

	namespace spray
	{

struct EmitterPoint;

/*! Particle renderer.
 * \ingroup Spray
 */
class T_DLLCLASS PointRenderer : public Object
{
	T_RTTI_CLASS;

public:
	explicit PointRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance);

	virtual ~PointRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const Plane& cameraPlane,
		const pointVector_t& points,
		float middleAge,
		float cullNearDistance,
		float fadeNearRange,
		float cameraOffset
	);

	void flush(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass
	);

	void setLodDistances(float lod1Distance, float lod2Distance)
	{
		m_lod1Distance = lod1Distance;
		m_lod2Distance = lod2Distance;
	}

	float getLod1Distance() const { return m_lod1Distance; }

	float getLod2Distance() const { return m_lod2Distance; }

private:
	struct Batch
	{
		uint32_t count;
		float distance;
		render::Shader* shader;
		uint32_t offset;
	};

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_structBuffer;
	float m_lod1Distance;
	float m_lod2Distance;
	EmitterPoint* m_point;
	int32_t m_pointOffset;
	
	AlignedVector< Batch > m_batches;
};

	}
}

