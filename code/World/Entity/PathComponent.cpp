/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/PathComponent.h"

namespace traktor::world
{
	namespace
	{
	
float findClosestRange(const TransformPath& path, bool closed, const Vector4& position, float Tstart, float Tend, int32_t steps)
{
	const float Tduration = Tend - Tstart;

	Scalar minLength = Scalar(std::numeric_limits< float >::max());
	float minTime = Tstart;

	for (int32_t i = 0; i <= steps; ++i)
	{
		const float t0 = float(i) / steps;
		const auto v0 = path.evaluate(Tstart + t0 * Tduration, closed);
		const Vector4 p0 = v0.transform().translation();
		const Scalar ln = (position - p0).length2();
		if (ln < minLength)
		{
			minLength = ln;
			minTime = Tstart + t0 * Tduration;
		}
	}

	return minTime;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PathComponent", PathComponent, IEntityComponent)

PathComponent::PathComponent(const TransformPath& path)
:	m_path(path)
,	m_closed(true)
{
}

void PathComponent::destroy()
{
}

void PathComponent::setOwner(Entity* owner)
{
}

void PathComponent::setTransform(const Transform& transform)
{
}

Aabb3 PathComponent::getBoundingBox() const
{
	return Aabb3();
}

void PathComponent::update(const UpdateParams& update)
{
}

Transform PathComponent::evaluate(float at) const
{
	return m_path.evaluate(at, m_closed).transform();
}


Transform PathComponent::evaluateDirectional(float at) const
{
	Matrix44 T = m_path.evaluate(at, m_closed).transform().toMatrix44();

	const Quaternion Qrot(T);

	const float c_atDelta = 0.001f;
	const Transform Tp = m_path.evaluate(at - c_atDelta, true).transform();
	const Transform Tn = m_path.evaluate(at + c_atDelta, true).transform();
	T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();

	T = T * rotateZ(Qrot.toEulerAngles().y());

	return Transform(T);
}

float PathComponent::estimateLength() const
{
	const float Tstart = m_path.getStartTime();
	const float Tend = m_path.getEndTime();
	const float Tduration = Tend - Tstart;

	float pathLength = 0.0f;
	for (int32_t i = 0; i < 100; ++i)
	{
		const float t0 = (float)i / 100.0f;
		const float t1 = (float)(i + 1) / 100.0f;

		const auto v0 = m_path.evaluate(Tstart + t0 * Tduration, m_closed);
		const auto v1 = m_path.evaluate(Tstart + t1 * Tduration, m_closed);

		const Vector4 p0 = v0.transform().translation();
		const Vector4 p1 = v1.transform().translation();

		pathLength += (p1 - p0).length();
	}

	return pathLength;
}

float PathComponent::findClosest(const Vector4& position) const
{
	const float Tstart = m_path.getStartTime();
	const float Tend = m_path.getEndTime();
	const float Tduration = Tend - Tstart;

	const float TstepA = (Tend - Tstart) / 100.0f;
	const float TstepB = (TstepA * 2.0f) / 10.0f;
	const float TstepC = (TstepB * 2.0f) / 10.0f;
	const float TstepD = (TstepC * 2.0f) / 10.0f;
	const float TstepE = (TstepD * 2.0f) / 10.0f;

	float Tmin;
	Tmin = findClosestRange(m_path, m_closed, position, Tstart, Tend, 100);
	Tmin = findClosestRange(m_path, m_closed, position, Tmin - TstepA, Tmin + TstepA, 10);
	Tmin = findClosestRange(m_path, m_closed, position, Tmin - TstepB, Tmin + TstepB, 10);
	Tmin = findClosestRange(m_path, m_closed, position, Tmin - TstepC, Tmin + TstepC, 10);
	Tmin = findClosestRange(m_path, m_closed, position, Tmin - TstepD, Tmin + TstepD, 10);
	Tmin = findClosestRange(m_path, m_closed, position, Tmin - TstepE, Tmin + TstepE, 10);

	while (Tmin < 0.0f)
		Tmin += Tduration;

	return Tmin;
}

}
