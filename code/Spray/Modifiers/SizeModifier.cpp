#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/SizeModifier.h"

#if defined(_PS3)
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SizeModifier", 0, SizeModifier, Modifier)

SizeModifier::SizeModifier()
:	m_adjustRate(0.0f)
{
}

#if defined(_PS3)
void SizeModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.transform = transform;
	job.deltaTime = deltaTime;
	job.pointsEA = (uintptr_t)(&points[0]);
	job.pointsCount = points.size();
	job.size.adjustRate = m_adjustRate;

	jobQueue->push(&job);
}
#else
void SizeModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
		points[i].size += m_adjustRate * deltaTime;
}
#endif

bool SizeModifier::serialize(ISerializer& s)
{
	return s >> Member< float >(L"adjustRate", m_adjustRate);
}

	}
}
