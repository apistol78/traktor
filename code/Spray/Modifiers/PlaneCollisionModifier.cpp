#include "Spray/Modifiers/PlaneCollisionModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

#if defined(_PS3)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PlaneCollisionModifier", 0, PlaneCollisionModifier, Modifier)

PlaneCollisionModifier::PlaneCollisionModifier()
:	m_plane(Vector4(0.0f, 1.0f, 0.0f), Scalar(0.0f))
,	m_restitution(1.0f)
{
}

#if defined(_PS3)
void PlaneCollisionModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	m_plane.normal().storeUnaligned(job.modifier.planeCollision.plane);
	job.modifier.planeCollision.plane[3] = m_plane.distance();
	job.modifier.planeCollision.restitution = m_restitution;

	jobQueue->push(&job);
}
#else
void PlaneCollisionModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		Scalar rv = dot3(m_plane.normal(), points[i].velocity);
		if (rv >= 0.0f)
			continue;

		Scalar rd = m_plane.distance(points[i].position);
		if (rd >= points[i].size)
			continue;

		points[i].velocity = -reflect(points[i].velocity, m_plane.normal()) * m_restitution;
	}
}
#endif

bool PlaneCollisionModifier::serialize(ISerializer& s)
{
	// @fixme Plane
	s >> Member< Scalar >(L"restitution", m_restitution);
	return true;
}

	}
}
