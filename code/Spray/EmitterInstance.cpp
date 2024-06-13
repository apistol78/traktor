/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Shader.h"
#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Types.h"
#include "Spray/MeshRenderer.h"
#include "Spray/Modifier.h"
#include "Spray/PointRenderer.h"
#include "Spray/Source.h"

#if !defined(__IOS__) && !defined(__ANDROID__)
#	define T_USE_UPDATE_JOBS
#endif

namespace traktor::spray
{
	namespace
	{

const float c_warmUpDeltaTime = 1.0f / 5.0f;
#if defined(__IOS__) || defined(__ANDROID__)
const uint32_t c_maxEmitPerUpdate = 4;
const uint32_t c_maxEmitSingleShot = 10;
#else
const uint32_t c_maxEmitPerUpdate = 64;
const uint32_t c_maxEmitSingleShot = 400;
#endif

const uint32_t c_maxAlive = c_maxEmitSingleShot;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EmitterInstance", EmitterInstance, Object)

EmitterInstance::EmitterInstance(const Emitter* emitter, float duration)
:	m_emitter(emitter)
,	m_transform(Transform::identity())
,	m_sortPlane(Vector4(0.0f, 0.0f, -1.0f), 0.0_simd)
,	m_totalTime(0.0f)
,	m_emitFraction(0.0f)
,	m_warm(false)
,	m_count(0)
,	m_skip(1)
{
	m_points.reserve(c_maxAlive);
	m_renderPoints.reserve(c_maxAlive);
}

EmitterInstance::~EmitterInstance()
{
	synchronize();
}

void EmitterInstance::update(Context& context, const Transform& transform, bool emit, bool singleShot)
{
	synchronize();

	// Warm up instance.
	if (!m_warm)
	{
		m_warm = true;
		m_transform = transform;

		if (m_emitter->getWarmUp() >= FUZZY_EPSILON)
		{
			Context warmContext;
			warmContext.deltaTime = c_warmUpDeltaTime;

			float time = 0.0f;
			for (; time < m_emitter->getWarmUp(); time += c_warmUpDeltaTime)
				update(warmContext, transform, true, false);

			warmContext.deltaTime = m_emitter->getWarmUp() - time + c_warmUpDeltaTime;
			if (warmContext.deltaTime >= FUZZY_EPSILON)
				update(warmContext, transform, true, false);
		}
	}

	const Vector4 lastPosition = m_transform.translation();
	m_transform = transform;

	// Erase dead particles.
	size_t size = m_points.size();
	if (!m_emitter->getEffect() || m_effectInstances.size() != m_points.size())
	{
		for (size_t i = 0; i < size; )
		{
			Point& point = m_points[i];
			if ((point.age += context.deltaTime) < point.maxAge)
				++i;
			else if (i < --size)
				point = m_points[size];
		}
		m_points.resize(size);
	}
	else
	{
		for (size_t i = 0; i < size; )
		{
			Point& point = m_points[i];
			if ((point.age += context.deltaTime) < point.maxAge)
				++i;
			else if (i < --size)
			{
				point = m_points[size];
				m_effectInstances[i] = m_effectInstances[size];
			}
		}
		m_points.resize(size);
		m_effectInstances.resize(size);
	}

	// Emit particles.
	if (emit)
	{
		const Source* source = m_emitter->getSource();
		if (source)
		{
			const uint32_t avail = (uint32_t)(m_points.capacity() - size);
			const Vector4 dm = (lastPosition - m_transform.translation()).xyz0();

			if (!singleShot)
			{
				const float emitVelocity = context.deltaTime > FUZZY_EPSILON ? source->getVelocityRate() * (dm.length() / context.deltaTime) : 0.0f;
				const float emitConstant = source->getConstantRate() * context.deltaTime;
				const float emit = emitVelocity + emitConstant + m_emitFraction;
				uint32_t emitCountFrame = uint32_t(emit);

				// Emit in multiple frames; estimate number of particles to emit.
				if (emitCountFrame > 0)
				{
					emitCountFrame = min< uint32_t >(emitCountFrame, avail, c_maxEmitPerUpdate);
					if (emitCountFrame > 0)
					{
						source->emit(
							context,
							m_emitter->worldSpace() ? m_transform : Transform::identity(),
							dm,
							emitCountFrame,
							*this
						);
					}
				}

				// Preserve fraction of non-emitted particles.
				m_emitFraction = emit - emitCountFrame;
			}
			else
			{
				// Single shot emit; emit all particles in one frame and then no more.
				const uint32_t emitCount = min< uint32_t >(uint32_t(source->getConstantRate()), avail, c_maxEmitSingleShot);
				if (emitCount > 0)
				{
					source->emit(
						context,
						m_emitter->worldSpace() ? m_transform : Transform::identity(),
						dm,
						emitCount,
						*this
					);
				}
			}
		}
	}

	m_totalTime += context.deltaTime;

	// Calculate bounding box; do this before modifiers as modifiers are executed
	// asynchronously.
	if ((m_count & 15) == 0)
	{
		m_boundingBox = Aabb3();
		const Scalar deltaTime16 = Scalar(context.deltaTime * 16.0f);
		for (const auto& point : m_points)
		{
			m_boundingBox.contain(point.position);
			m_boundingBox.contain(point.position + point.velocity * deltaTime16);
		}
		m_boundingBox = m_boundingBox.expand(1.0_simd);
		if (!m_emitter->worldSpace())
			m_boundingBox = m_boundingBox.transform(transform);
	}

	// Update particles on CPU
	size = m_points.size();
#if defined(T_USE_UPDATE_JOBS)
	// Execute modifiers.
	if (size >= 64)
	{
		JobManager& jobManager = JobManager::getInstance();
		m_job = jobManager.add([=, this](){
			updateTask(context.deltaTime);
		});
	}
	else
		updateTask(context.deltaTime);
#else
	updateTask(context.deltaTime);
#endif
}

void EmitterInstance::render(
	render::handle_t technique,
	PointRenderer* pointRenderer,
	MeshRenderer* meshRenderer,
	TrailRenderer* trailRenderer,
	const Transform& transform,
	const Vector4& cameraPosition,
	const Plane& cameraPlane
)
{
	T_ASSERT(m_count > 0);

	synchronize();

	m_sortPlane = cameraPlane;

	const float distance = cameraPlane.distance(transform.translation());
	if (distance > pointRenderer->getLod2Distance())
		m_skip = 4;
	else if (distance > pointRenderer->getLod1Distance())
		m_skip = 2;
	else
		m_skip = 1;

	if (m_renderPoints.empty())
		return;

	if (
		m_emitter->getShader() &&
		m_emitter->getShader()->hasTechnique(technique)
	)
	{
		pointRenderer->render(
			m_emitter->getShader(),
			cameraPlane,
			m_renderPoints,
			m_emitter->getMiddleAge(),
			m_emitter->getCullNearDistance(),
			m_emitter->getFadeNearRange(),
			m_emitter->getViewOffset()
		);
	}

	if (
		m_emitter->getMesh() &&
		distance < m_emitter->getCullMeshDistance()
	)
	{
		meshRenderer->render(
			m_emitter->getMesh(),
			m_emitter->meshOrientationFromVelocity(),
			m_renderPoints
		);
	}

	if (
		m_emitter->getEffect()
	)
	{
		T_ASSERT(m_renderPoints.size() == m_effectInstances.size());
		for (uint32_t i = 0; i < m_renderPoints.size(); ++i)
		{
			if (!m_effectInstances[i])
				continue;

			const Point& point = m_renderPoints[i];

			m_effectInstances[i]->render(
				technique,
				pointRenderer,
				meshRenderer,
				trailRenderer,
				Transform(
					point.position,
					Quaternion(Vector4(0.0f, 1.0f, 0.0f), point.velocity)
				),
				cameraPosition,
				cameraPlane
			);
		}
	}
}

void EmitterInstance::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	if (m_job)
	{
		m_job->wait();
		m_job = nullptr;
	}
#endif
}

void EmitterInstance::updateTask(float deltaTime)
{
	const Transform updateTransform = m_emitter->worldSpace() ? m_transform : Transform::identity();
	const Scalar deltaTimeScalar(deltaTime);

	for (auto modifier : m_emitter->getModifiers())
	{
		modifier->update(
			deltaTimeScalar,
			updateTransform,
			m_points,
			0,
			m_points.size()
		);
	}

	m_renderPoints.resize(0);

	for (uint32_t i = 0; i < m_points.size(); i += m_skip)
		m_renderPoints.push_back(m_points[i]);

	if (!m_emitter->worldSpace())
	{
		for (uint32_t i = 0; i < m_renderPoints.size(); ++i)
		{
			m_renderPoints[i].position = m_transform * m_renderPoints[i].position;
			m_renderPoints[i].velocity = m_transform * m_renderPoints[i].velocity;
		}
	}

	// \note Do not sort furthest lod.
	if (
		!m_renderPoints.empty() &&
		m_emitter->getSort() &&
		m_skip < 4
	)
	{
		std::sort(m_renderPoints.begin(), m_renderPoints.end(), [=, this](const Point& lh, const Point& rh) {
			return (bool)(m_sortPlane.distance(lh.position) > m_sortPlane.distance(rh.position));
		});
	}

	if (m_emitter->getEffect())
	{
		// Create new effect instances for each new particle point.
		const uint32_t size = uint32_t(m_effectInstances.size());
		m_effectInstances.resize(m_renderPoints.size());
		for (uint32_t i = size; i < m_renderPoints.size(); ++i)
			m_effectInstances[i] = m_emitter->getEffect()->createInstance();

		// Update child effect instances.
		Context childContext;
		childContext.deltaTime = deltaTime;

		const float duration = m_emitter->getEffect()->getDuration();
		for (uint32_t i = 0; i < m_renderPoints.size(); ++i)
		{
			if (!m_effectInstances[i])
				continue;

			const Point& point = m_renderPoints[i];

			m_effectInstances[i]->update(
				childContext,
				Transform(
					point.position,
					Quaternion(Vector4(0.0f, 1.0f, 0.0f), point.velocity)
				),
				m_effectInstances[i]->getTime() < duration
			);
		}
	}

	m_count++;
}

}
