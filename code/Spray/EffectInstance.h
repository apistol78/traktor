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
#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"
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

namespace traktor::spray
{

struct Context;
class Effect;
class EffectLayerInstance;
class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! Effect instance.
 * \ingroup Spray
 */
class T_DLLCLASS EffectInstance : public Object
{
	T_RTTI_CLASS;

public:
	explicit EffectInstance(const Effect* effect);

	void update(Context& context, const Transform& transform, bool enable);

	void synchronize();

	void render(
		render::handle_t technique,
		render::RenderContext* renderContext,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer,
		const Transform& transform,
		const Vector4& cameraPosition,
		const Plane& cameraPlane
	) const;

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setLoopEnable(bool loopEnable) { m_loopEnable = loopEnable; }

	bool getLoopEnable() const { return m_loopEnable; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

	const RefArray< EffectLayerInstance >& getLayerInstances() const { return m_layerInstances; }

private:
	friend class Effect;

	Ref< const Effect > m_effect;
	float m_time;
	bool m_loopEnable;
	Aabb3 m_boundingBox;
	RefArray< EffectLayerInstance > m_layerInstances;
};

}
