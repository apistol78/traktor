#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/Sequence.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectLayer", 1, EffectLayer, ISerializable)

EffectLayer::EffectLayer()
:	m_time(0.0f)
,	m_duration(0.0f)
{
}

bool EffectLayer::bind(resource::IResourceManager* resourceManager)
{
	if (m_emitter && !m_emitter->bind(resourceManager))
		return false;

	if (m_sequence && !m_sequence->bind(resourceManager))
		return false;

	return true;
}

Ref< EffectLayerInstance > EffectLayer::createInstance() const
{
	Ref< EmitterInstance > emitterInstance;
	Ref< SequenceInstance > sequenceInstance;

	if (m_emitter)
	{
		emitterInstance = m_emitter->createInstance();
		if (!emitterInstance)
			return 0;
	}

	if (m_sequence)
	{
		sequenceInstance = m_sequence->createInstance();
		if (!sequenceInstance)
			return 0;
	}

	return new EffectLayerInstance(
		this,
		emitterInstance,
		sequenceInstance
	);
}

bool EffectLayer::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", m_time);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRef< Emitter >(L"emitter", m_emitter);

	if (s.getVersion() >= 1)
		s >> MemberRef< Sequence >(L"sequence", m_sequence);

	return true;
}

	}
}
