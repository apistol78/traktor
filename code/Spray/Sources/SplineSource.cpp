#include "Core/Math/Hermite.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/EmitterInstance.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/Sources/SplineSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SplineSource", 0, SplineSource, Source)

SplineSource::SplineSource()
:	m_mode(MdRandom)
,	m_sequentialSpeed(1.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

bool SplineSource::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

void SplineSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	if (m_keys.empty())
		return;

	float time = m_keys.back().T;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		float at = m_mode == MdRandom ? context.random.nextFloat() * time : std::fmod(emitterInstance.getTotalTime() * m_sequentialSpeed, time);
		Vector4 position = transform * Hermite< Key, Scalar, Vector4 >(&m_keys[0], m_keys.size()).evaluate(Scalar(at));

		point->position = position;
		point->velocity = context.random.nextUnit() * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->color = Vector4::one();
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();

		++point;
	}
}

bool SplineSource::serialize(ISerializer& s)
{
	const MemberEnum< Mode >::Key c_Mode_Keys[] =
	{
		{ L"MdRandom", MdRandom },
		{ L"MdSequential", MdSequential },
		{ 0 }
	};

	if (!Source::serialize(s))
		return false;

	s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
	s >> MemberEnum< Mode >(L"mode", m_mode, c_Mode_Keys);
	s >> Member< float >(L"sequentialSpeed", m_sequentialSpeed);
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);

	return true;
}

bool SplineSource::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> Member< Vector4 >(L"value", value, AttributePoint());
	return true;
}

	}
}
