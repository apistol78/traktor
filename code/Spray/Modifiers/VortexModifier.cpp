#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/VortexModifier.h"

#if defined(_PS3)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobVortexModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobVortexModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobVortexModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobVortexModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.VortexModifier", 1, VortexModifier, Modifier)

VortexModifier::VortexModifier()
:	m_axis(0.0f, 1.0f, 0.0f, 0.0f)
,	m_tangentForce(0.0f)
,	m_normalConstantForce(0.0f)
,	m_normalDistance(0.0f)
,	m_normalDistanceForce(0.0f)
,	m_world(true)
{
}

#if defined(_PS3)
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
	Vector4 center = transform.translation();

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

bool VortexModifier::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"axis", m_axis);
	s >> Member< Scalar >(L"tangentForce", m_tangentForce);
	s >> Member< Scalar >(L"normalConstantForce", m_normalConstantForce);
	s >> Member< Scalar >(L"normalDistance", m_normalDistance);
	s >> Member< Scalar >(L"normalDistanceForce", m_normalDistanceForce);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"world", m_world);
	return true;
}

	}
}
