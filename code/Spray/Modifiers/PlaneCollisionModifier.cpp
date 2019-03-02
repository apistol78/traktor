#include "Spray/Modifiers/PlaneCollisionModifier.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobPlaneCollisionModifier_bin_size;

#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PlaneCollisionModifier", PlaneCollisionModifier, Modifier)

PlaneCollisionModifier::PlaneCollisionModifier(const Plane& plane, float radius, float restitution)
:	m_plane(plane)
,	m_radius(radius)
,	m_restitution(restitution)
{
}

#if defined(T_MODIFIER_USE_PS3_SPURS)
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
	m_plane.normal().storeAligned(job.modifier.planeCollision.plane);
	job.modifier.planeCollision.plane[3] = m_plane.distance();
	job.modifier.planeCollision.restitution = m_restitution;

	jobQueue->push(&job);
}
#else
void PlaneCollisionModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Plane planeW = transform.toMatrix44() * m_plane;
	Vector4 center = transform.translation();

	for (size_t i = first; i < last; ++i)
	{
		Scalar rv = dot3(planeW.normal(), points[i].velocity);
		if (rv >= 0.0f)
			continue;

		Scalar rd = planeW.distance(points[i].position);
		if (rd >= points[i].size)
			continue;

		if ((points[i].position - center).length2() >= m_radius)
			continue;

		points[i].velocity = -reflect(points[i].velocity, m_plane.normal()) * m_restitution;
	}
}
#endif

	}
}
