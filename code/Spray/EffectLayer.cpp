#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.EffectLayer", EffectLayer, Serializable)

EffectLayer::EffectLayer()
:	m_time(0.0f)
,	m_duration(0.0f)
{
}

EffectLayerInstance* EffectLayer::createInstance(resource::IResourceManager* resourceManager) const
{
	if (!m_emitter)
		return 0;

	Ref< EmitterInstance > emitterInstance = m_emitter->createInstance(resourceManager);
	if (!emitterInstance)
		return 0;

	return gc_new< EffectLayerInstance >(
		this,
		emitterInstance
	);
}

bool EffectLayer::serialize(Serializer& s)
{
	s >> Member< float >(L"time", m_time);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRef< Emitter >(L"emitter", m_emitter);
	return true;
}

	}
}
