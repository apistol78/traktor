/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Math/Hermite.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/RiverComponentData.h"
#include "World/IWorldRenderPass.h"

namespace traktor::terrain
{
	namespace
	{

struct ControlPointAccessor
{
	typedef float time_t;
	typedef RiverComponentData::ControlPoint key_t;

	static time_t time(const key_t* keys, size_t nkeys, const key_t& key)
	{
		int32_t index = int32_t(&key - keys);
		return index / time_t(nkeys - 1);
	}

	static const key_t& value(const key_t& key)
	{
		return key;
	}

	static key_t combine(
		time_t t,
		const key_t& v0, time_t w0,
		const key_t& v1, time_t w1,
		const key_t& v2, time_t w2,
		const key_t& v3, time_t w3
	)
	{
		key_t c;
		c.width = v0.width * w0 + v1.width * w1 + v2.width * w2 + v3.width * w3;
		c.position = v0.position * Scalar(w0) + v1.position * Scalar(w1) + v2.position * Scalar(w2) + v3.position * Scalar(w3);
		c.tension = v0.tension * w0 + v1.tension * w1 + v2.tension * w2 + v3.tension * w3;
		return c;
	}
};

typedef Hermite< RiverComponentData::ControlPoint, RiverComponentData::ControlPoint, ControlPointAccessor > hermite_t;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RiverComponent", RiverComponent, world::IEntityComponent)

bool RiverComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const RiverComponentData& data)
{
	const auto& path = data.getPath();
	if (path.size() < 2)
		return false;

	float length = 0.0f;
	for (auto it = path.begin() + 1; it != path.end(); ++it)
		length += (it->position - (it - 1)->position).length();

	const float dT = std::max(1.0f / length, 0.001f);

	AlignedVector< Vector4 > silouette;

	hermite_t h(path.c_ptr(), path.size());
	RiverComponentData::ControlPoint pc = h.evaluate(0.0f);
	for (float T = dT; T <= 1.0f; )
	{
		const RiverComponentData::ControlPoint pn = h.evaluate(T);

		const Vector4 d = pn.position - pc.position;
		const Vector4 e = cross(d, Vector4(0.0f, 1.0f, 0.0f, 0.0f)).normalized();

		silouette.push_back((pc.position - e * Scalar(pc.width / 2.0f)).xyz0() + Vector4(0.0f, 0.0f, 0.0f, T));
		silouette.push_back((pc.position + e * Scalar(pc.width / 2.0f)).xyz0() + Vector4(0.0f, 0.0f, 0.0f, T));

		pc = pn;

		T += dT * pn.tension;
	}

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat4, 0));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat2, 4 * sizeof(float)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		(uint32_t)silouette.size() * 6 * sizeof(float),
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = (float*)m_vertexBuffer->lock();
	if (!vertex)
		return false;

	float u = 0.0f;
	for (const auto& s : silouette)
	{
		s.xyz1().storeUnaligned(vertex);
		vertex += 4;

		*vertex++ = u;
		*vertex++ = data.getTileFactorV() * s.w();

		u = 1.0f - u;
	}

	m_vertexBuffer->unlock();

	const uint32_t triangleCount = (uint32_t)(silouette.size() - 2);

	m_indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		triangleCount * 3 * sizeof(uint16_t),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = (uint16_t*)m_indexBuffer->lock();
	if (!index)
		return false;

	for (uint16_t i = 0; i < (uint16_t)triangleCount; i += 2)
	{
		*index++ = i + 0;
		*index++ = i + 1;
		*index++ = i + 2;
		*index++ = i + 3;
		*index++ = i + 1;
		*index++ = i + 2;
	}

	m_indexBuffer->unlock();

	m_primitives = render::Primitives::setIndexed(
		render::PrimitiveType::Triangles,
		0,
		triangleCount
	);

	if (!resourceManager->bind(data.getShader(), m_shader))
		return false;

	return true;
}

void RiverComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
}

void RiverComponent::setOwner(world::Entity* owner)
{
}

void RiverComponent::setTransform(const Transform& transform)
{
}

Aabb3 RiverComponent::getBoundingBox() const
{
	return Aabb3();
}

void RiverComponent::update(const world::UpdateParams& update)
{
}

void RiverComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;			

	auto renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"River");
	renderBlock->distance = 0.0f;
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

}
