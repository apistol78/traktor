#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/GravityModifier.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"

#	if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Spray_JobGravityModifier_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobGravityModifier_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobGravityModifier_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobGravityModifier_bin_size;

#	else

extern char _binary_jqjob_Traktor_Spray_JobGravityModifier_d_bin_start[];
extern char _binary_jqjob_Traktor_Spray_JobGravityModifier_d_bin_size[];

static char* job_start = _binary_jqjob_Traktor_Spray_JobGravityModifier_d_bin_start;
static char* job_size = _binary_jqjob_Traktor_Spray_JobGravityModifier_d_bin_size;

#	endif
#endif

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.GravityModifier", 1, GravityModifier, Modifier)

GravityModifier::GravityModifier()
:	m_gravity(0.0f, 0.0f, 0.0f, 0.0f)
,	m_world(true)
{
}

#if defined(T_MODIFIER_USE_PS3_SPURS)
void GravityModifier::update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const
{
	JobModifierUpdate job;

	__builtin_memset(&job, 0, sizeof(JobModifierUpdate));
	job.header.eaBinary = (uintptr_t)job_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_size);

	job.common.transform = transform;
	job.common.deltaTime = deltaTime;
	job.common.pointsEA = (uintptr_t)(&points[0]);
	job.common.pointsCount = points.size();
	job.modifier.gravity.world = m_world;
	m_gravity.storeAligned(job.modifier.gravity.gravity);

	jobQueue->push(&job);
}
#else
void GravityModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Vector4 gravity = (m_world ? m_gravity : transform * m_gravity) * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].velocity += gravity * Scalar(points[i].inverseMass);
}
#endif

bool GravityModifier::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"gravity", m_gravity, AttributeDirection());
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"world", m_world);
	return true;
}

	}
}
