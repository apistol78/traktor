#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.spray.Effect", Effect, Serializable)

Effect::Effect()
:	m_duration(0.0f)
,	m_loopStart(0.0f)
,	m_loopEnd(0.0f)
{
}

EffectInstance* Effect::createInstance() const
{
	Ref< EffectInstance > effectInstance = gc_new< EffectInstance >(this);
	for (RefArray< EffectLayer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Ref< EffectLayerInstance > layerInstance = (*i)->createInstance();
		if (layerInstance)
			effectInstance->m_layerInstances.push_back(layerInstance);
	}
	return effectInstance;
}

bool Effect::serialize(Serializer& s)
{
	s >> Member< float >(L"duration", m_duration);
	s >> Member< float >(L"loopStart", m_loopStart);
	s >> Member< float >(L"loopEnd", m_loopEnd);
	s >> MemberRefArray< EffectLayer >(L"layers", m_layers);
	return true;
}

	}
}
