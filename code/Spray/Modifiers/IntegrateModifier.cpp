#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/IntegrateModifier.h"

#if defined(_PS3)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobIntegrateModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobIntegrateModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobIntegrateModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobIntegrateModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobIntegrateModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobIntegrateModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobIntegrateModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobIntegrateModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.IntegrateModifier", 0, IntegrateModifier, Modifier)

IntegrateModifier::IntegrateModifier()
:	m_timeScale(1.0f)
{
}

#if defined(_PS3)
void IntegrateModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	job.modifier.integrate.timeScale = m_timeScale;

	jobQueue->push(&job);
}
#else
void IntegrateModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Scalar scaledDeltaTime = deltaTime * m_timeScale;
	for (size_t i = first; i < last; ++i)
	{
		points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
		points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
	}
}
#endif

bool IntegrateModifier::serialize(ISerializer& s)
{
	return s >> Member< Scalar >(L"timeScale", m_timeScale);
}

	}
}
