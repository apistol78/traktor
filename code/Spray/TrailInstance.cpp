/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Render/Shader.h"
#include "Spray/TrailInstance.h"
#include "Spray/TrailRenderer.h"

namespace traktor::spray
{
	namespace
	{

const Scalar c_lengthThreshold(0.1f);
const Scalar c_breakThreshold(10.0f);

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.TrailInstance", TrailInstance, Object)

TrailInstance::TrailInstance(
	const resource::Proxy< render::Shader >& shader,
	float width,
	float age,
	float lengthThreshold,
	float breakThreshold
)
:	m_shader(shader)
,	m_width(width)
,	m_age(age)
,	m_lengthThreshold(lengthThreshold)
,	m_breakThreshold(breakThreshold)
,	m_last(Vector4::zero())
,	m_time(Vector4::zero())
{
}

void TrailInstance::update(Context& context, const Transform& transform, bool enable)
{
	while (!m_points.empty())
	{
		Scalar w = (m_time - m_points.front()).w();
		if (w < m_age)
			break;
		m_points.pop_front();
	}

	if (enable)
	{
		const Vector4 position = transform.translation().xyz1();

		if (m_points.empty())
		{
			m_last = position;
			m_points.push_back(position.xyz0() + m_time);
			m_boundingBox.contain(position);
		}

		if (m_breakThreshold > FUZZY_EPSILON && m_lengthThreshold > FUZZY_EPSILON)	// Breakable
		{
			const Scalar ln = (position - m_last).length2();
			if (ln >= m_lengthThreshold * m_lengthThreshold)
			{
				if (ln >= m_breakThreshold * m_breakThreshold)
					m_points.clear();

				m_points.push_back(position.xyz0() + m_time);
				m_boundingBox.contain(position);

				m_last = position;
			}
			else
				m_points.back() = position.xyz0() + m_time;
		}
		else if (m_lengthThreshold > FUZZY_EPSILON)	// Segmented by distance
		{
			const Vector4 direction = (position - m_last).xyz0();
			Scalar ln = direction.length();

			const int32_t nsteps = int32_t(ln / m_lengthThreshold);
			if (nsteps > 0)
			{
				m_points.pop_back();

				const Scalar lnt(m_lengthThreshold);
				const Vector4 step = direction * lnt / ln;
				for (int32_t i = 0; i < nsteps; ++i)
				{
					m_points.push_back(m_last.xyz0() + m_time + Vector4(0.0f, 0.0f, 0.0f, (context.deltaTime * i) / nsteps));
					m_boundingBox.contain(m_last);
					m_last += step;
					ln -= lnt;
				}

				m_last -= step;
			}

			m_points.back() = position.xyz0() + m_time;
		}
		else // Always add points.
		{
			const Vector4 direction = (position - m_last).xyz0();
			const Scalar ln = direction.length();

			if (ln > FUZZY_EPSILON)
				m_points.push_back(position.xyz0() + m_time);

			m_last = position;
		}

		m_time += Vector4(0.0f, 0.0f, 0.0f, context.deltaTime);
	}
	else
		m_time += Vector4(0.0f, 0.0f, 0.0f, context.deltaTime);
}

void TrailInstance::render(render::handle_t technique, TrailRenderer* trailRenderer, const Transform& transform, const Vector4& cameraPosition, const Plane& cameraPlane)
{
	if (m_shader->hasTechnique(technique))
		trailRenderer->render(
			m_shader,
			m_points,
			cameraPosition,
			cameraPlane,
			m_width,
			m_time.w(),
			m_age
		);
}

}
