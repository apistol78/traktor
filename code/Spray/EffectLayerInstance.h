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
#include "Core/Math/Transform.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Aabb3.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;

}

namespace traktor::world
{

class IEntityEventInstance;

}

namespace traktor::spray
{

struct Context;
class EffectLayer;
class IEmitterInstance;
class MeshRenderer;
class PointRenderer;
class SequenceInstance;
class TrailInstance;
class TrailRenderer;

class T_DLLCLASS EffectLayerInstance : public Object
{
	T_RTTI_CLASS;

public:
	explicit EffectLayerInstance(
		const EffectLayer* layer,
		IEmitterInstance* emitterInstance,
		TrailInstance* trailInstance,
		SequenceInstance* sequenceInstance
	);

	virtual ~EffectLayerInstance();

	void update(Context& context, const Transform& transform, float time, bool enable);

	void synchronize();

	void render(
		render::handle_t technique,
		render::RenderContext* renderContext,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		float time
	) const;

	Aabb3 getBoundingBox() const;

	const IEmitterInstance* getEmitterInstance() const { return m_emitterInstance; }

	const TrailInstance* getTrailInstance() const { return m_trailInstance; }

	const SequenceInstance* getSequenceInstance() const { return m_sequenceInstance; }

	const EffectLayer* getLayer() const { return m_layer; }

private:
	Ref< const EffectLayer > m_layer;
	Ref< IEmitterInstance > m_emitterInstance;
	Ref< TrailInstance > m_trailInstance;
	Ref< SequenceInstance > m_sequenceInstance;
	Ref< world::IEntityEventInstance > m_triggerInstance;
	float m_start;
	float m_end;
	bool m_singleShotFired;
	bool m_enable;
};

}
