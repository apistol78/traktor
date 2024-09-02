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
class IRenderSystem;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class Emitter;

/*! Emitter instance.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterInstanceGPU : public IEmitterInstance
{
	T_RTTI_CLASS;

public:
	static Ref< EmitterInstanceGPU > createInstance(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager, const Emitter* emitter, float duration);

	virtual ~EmitterInstanceGPU();

	virtual void update(Context& context, const Transform& transform, bool emit, bool singleShot) override final;

	virtual void setup() override final;

	virtual void render(
		render::handle_t technique,
		render::RenderContext* renderContext,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane
	) override final;

	virtual void synchronize() const override final;

	virtual Aabb3 getBoundingBox() const override final { return m_boundingBox; }

private:
	Ref< const Emitter > m_emitter;
	resource::Proxy< render::Shader > m_shaderLifetime;
	Ref< render::Buffer > m_headBuffer;
	Ref< render::Buffer > m_pointBuffer;
	Aabb3 m_boundingBox;

	int32_t m_pendingEmit = 0;
	float m_pendingDeltaTime = 0.0f;
	float m_emitFraction = 0.0f;
	bool m_needLifetimeUpdate = false;

	explicit EmitterInstanceGPU(const Emitter* emitter);
};

}
