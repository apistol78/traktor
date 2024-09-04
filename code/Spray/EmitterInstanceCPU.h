/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Thread/Job.h"
#include "Spray/IEmitterInstance.h"
#include "Spray/Modifier.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

}

namespace traktor::spray
{

class EffectInstance;
class Emitter;

/*! Emitter instance.
 * \ingroup Spray
 */
class T_DLLCLASS EmitterInstanceCPU : public IEmitterInstance
{
	T_RTTI_CLASS;

public:
	static Ref< EmitterInstanceCPU > createInstance(const Emitter* emitter, float duration);

	virtual ~EmitterInstanceCPU();

	virtual void update(Context& context, const Transform& transform, bool emit, bool singleShot) override final;

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

	void setTotalTime(float totalTime) { m_totalTime = totalTime; }

	float getTotalTime() const { return m_totalTime; }

	void reservePoints(uint32_t npoints) { m_points.reserve(m_points.size() + npoints); }

	const pointVector_t& getPoints() const { return m_points; }

	Point* addPoints(uint32_t points)
	{
		const uint32_t offset = uint32_t(m_points.size());
		m_points.resize(offset + points);
		return &m_points[offset];
	}

private:
	Ref< const Emitter > m_emitter;
	Transform m_transform;
	Plane m_sortPlane;
	pointVector_t m_points;
	pointVector_t m_renderPoints;
	RefArray< EffectInstance > m_effectInstances;
	float m_totalTime;
	float m_emitFraction;
	bool m_warm;
	Aabb3 m_boundingBox;
	uint32_t m_count;
	uint32_t m_skip;
	mutable Ref< Job > m_job;

	explicit EmitterInstanceCPU(const Emitter* emitter, float duration);

	void updateTask(float deltaTime);
};

}
