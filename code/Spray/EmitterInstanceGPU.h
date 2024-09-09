/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/IEmitterInstance.h"

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
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class Emitter;
class GPUBufferPool;

// Buffer containing information about particle system instance.
// Suitable for indirect draw thus IndexedIndirectDraw must be at top.
struct Head
{
	render::IndexedIndirectDraw indirectDraw;
	render::IndirectCompute indirectCompute;
	int32_t capacity;
	int32_t alive;
	Vector4 modifiers[16];
};

/*! Emitter instance.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterInstanceGPU : public IEmitterInstance
{
	T_RTTI_CLASS;

public:
	static Ref< EmitterInstanceGPU > createInstance(resource::IResourceManager* resourceManager, GPUBufferPool* gpuBufferPool, const Emitter* emitter, uint32_t capacity, float duration);

	virtual ~EmitterInstanceGPU();

	virtual void update(Context& context, const Transform& transform, bool emit, bool singleShot) override final;

	virtual void render(
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		render::RenderContext* renderContext,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform
	) override final;

	virtual void synchronize() const override final;

	virtual Aabb3 getBoundingBox() const override final { return m_boundingBox; }

private:
	struct Update
	{
		float deltaTime;
		int32_t emit;
		int32_t seed;
	};

	Ref< const Emitter > m_emitter;
	resource::Proxy< render::Shader > m_shaderLifetime;
	resource::Proxy< render::Shader > m_shaderEvolve;
	resource::Proxy< render::Shader > m_shaderSource;
	Ref< GPUBufferPool > m_gpuBufferPool;
	Ref< render::Buffer > m_headBuffer;
	Ref< render::Buffer > m_pointBuffer;
	Aabb3 m_boundingBox;
	Transform m_transform;
	float m_emitFraction = 0.0f;
	AlignedVector< Update > m_updates;

	explicit EmitterInstanceGPU(const Emitter* emitter);
};

}
