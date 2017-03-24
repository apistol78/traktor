#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.illuminate.IlluminateEntityData", 3, IlluminateEntityData, world::EntityData)

IlluminateEntityData::IlluminateEntityData()
:	m_seedGuid(Guid::create())
,	m_occlusion(true)
,	m_directLighting(true)
,	m_indirectLighting(true)
,	m_lumelDensity(8.0f)
,	m_pointLightRadius(0.02f)
,	m_shadowSamples(64)
,	m_probeSamples(64)
,	m_probeCoeff(0.001f)
,	m_probeSpread(0.5f)
,	m_probeShadowSpread(0.5f)
,	m_directConvolveRadius(0)
,	m_indirectTraceSamples(64)
,	m_indirectTraceIterations(1)
,	m_indirectConvolveRadius(0)
,	m_highDynamicRange(false)
,	m_compressLightMap(true)
{
}

void IlluminateEntityData::addEntityData(world::EntityData* entityData)
{
	T_ASSERT (std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void IlluminateEntityData::removeEntityData(world::EntityData* entityData)
{
	RefArray< world::EntityData >::iterator i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void IlluminateEntityData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void IlluminateEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = transform * getTransform().inverse();
	for (RefArray< world::EntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(deltaTransform * currentTransform);
	}
	world::EntityData::setTransform(transform);
}

void IlluminateEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	
	s >> Member< Guid >(L"seedGuid", m_seedGuid);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"occlusion", m_occlusion);

	s >> Member< bool >(L"directLighting", m_directLighting);
	s >> Member< bool >(L"indirectLighting", m_indirectLighting);

	if (s.getVersion() >= 2)
		s >> Member< float >(L"lumelDensity", m_lumelDensity);

	s >> Member< float >(L"pointLightRadius", m_pointLightRadius);
	s >> Member< int32_t >(L"shadowSamples", m_shadowSamples, AttributeRange(0));

	if (s.getVersion() >= 1)
	{
		s >> Member< int32_t >(L"probeSamples", m_probeSamples, AttributeRange(1));
		s >> Member< float >(L"probeCoeff", m_probeCoeff, AttributeRange(0.0f));
		s >> Member< float >(L"probeSpread", m_probeSpread, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"probeShadowSpread", m_probeShadowSpread, AttributeRange(0.0f, 1.0f));
	}

	s >> Member< int32_t >(L"directConvolveRadius", m_directConvolveRadius, AttributeRange(0));
	s >> Member< int32_t >(L"indirectTraceSamples", m_indirectTraceSamples, AttributeRange(1));
	s >> Member< int32_t >(L"indirectTraceIterations", m_indirectTraceIterations, AttributeRange(1));
	s >> Member< int32_t >(L"indirectConvolveRadius", m_indirectConvolveRadius, AttributeRange(0));
	s >> Member< bool >(L"highDynamicRange", m_highDynamicRange);
	s >> Member< bool >(L"compressLightMap", m_compressLightMap);
	s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
}

	}
}
