/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/Modifiers/VortexModifier.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_size;

#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.VortexModifier", VortexModifier, Modifier)

VortexModifier::VortexModifier(
	const Vector4& axis,
	float tangentForce,
	float normalConstantForce,
	float normalDistance,
	float normalDistanceForce,
	bool world
)
:	m_axis(axis)
,	m_tangentForce(tangentForce)
,	m_normalConstantForce(normalConstantForce)
,	m_normalDistance(normalDistance)
,	m_normalDistanceForce(normalDistanceForce)
,	m_world(world)
{
}

#if defined(T_MODIFIER_USE_PS3_SPURS)
void VortexModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	m_axis.storeAligned(job.modifier.vortex.axis);
	job.modifier.vortex.tangentForce = m_tangentForce;
	job.modifier.vortex.normalConstantForce = m_normalConstantForce;
	job.modifier.vortex.normalDistance = m_normalDistance;
	job.modifier.vortex.normalDistanceForce = m_normalDistanceForce;

	jobQueue->push(&job);
}
#else
void VortexModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Vector4 axis = m_world ? m_axis : transform * m_axis;
	Vector4 center = m_world ? transform.translation() : Vector4::origo();

	for (size_t i = first; i < last; ++i)
	{
		Vector4 pc = points[i].position - center;

		// Project onto plane.
		Scalar d = dot3(pc, axis);
		pc -= axis * d;

		// Calculate tangent vector.
		Scalar distance = pc.length();
		Vector4 n = pc / distance;
		Vector4 t = cross(axis, n).normalized();

		// Adjust velocity from this tangent.
		points[i].velocity += (
			t * m_tangentForce +
			n * (m_normalConstantForce + (distance - m_normalDistance) * m_normalDistanceForce)
		) * Scalar(points[i].inverseMass) * deltaTime;
	}
}
#endif

	}
}
