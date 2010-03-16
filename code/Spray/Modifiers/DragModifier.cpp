#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/DragModifier.h"

#if defined(_PS3)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobDragModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobDragModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobDragModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobDragModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobDragModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobDragModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobDragModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobDragModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DragModifier", 0, DragModifier, Modifier)

DragModifier::DragModifier()
:	m_linearDrag(0.0f)
,	m_angularDrag(0.0f)
{
}

#if defined(_PS3)
void DragModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	job.modifier.drag.linearDrag = m_linearDrag;
	job.modifier.drag.angularDrag = m_angularDrag;

	jobQueue->push(&job);
}
#else
void DragModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		points[i].velocity *= Scalar(1.0f) - m_linearDrag * deltaTime;
		points[i].angularVelocity *= 1.0f - m_angularDrag * deltaTime;
	}
}
#endif

bool DragModifier::serialize(ISerializer& s)
{
	s >> Member< Scalar >(L"linearDrag", m_linearDrag);
	s >> Member< float >(L"angularDrag", m_angularDrag);
	return true;
}

	}
}
