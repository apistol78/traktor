#include "Spray/Modifiers/SizeModifier.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobSizeModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobSizeModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobSizeModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobSizeModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobSizeModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobSizeModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobSizeModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobSizeModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SizeModifier", SizeModifier, Modifier)

SizeModifier::SizeModifier(float adjustRate)
:	m_adjustRate(adjustRate)
{
}

#if defined(T_MODIFIER_USE_PS3_SPURS)
void SizeModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	job.modifier.size.adjustRate = m_adjustRate;

	jobQueue->push(&job);
}
#else
void SizeModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	float deltaSize = m_adjustRate * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].size += deltaSize;
}
#endif

	}
}
