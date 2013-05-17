#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Resound/IGrainData.h"
#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 5, BankResource, ISoundResource)

BankResource::BankResource()
:	m_volume(1.0f)
,	m_presence(0.0f)
,	m_presenceRate(1.0f)
,	m_range(0.0f)
{
}

BankResource::BankResource(const RefArray< IGrainData >& grains, const std::wstring& category, float volume, float presence, float presenceRate, float range)
:	m_grains(grains)
,	m_category(category)
,	m_volume(volume)
,	m_presence(presence)
,	m_presenceRate(presenceRate)
,	m_range(range)
{
}

Ref< Sound > BankResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = m_grains[i]->createInstance(resourceManager);
		if (!grains[i])
			return 0;
	}

	return new Sound(
		new BankBuffer(grains),
		getParameterHandle(m_category),
		m_volume,
		m_presence,
		m_presenceRate,
		m_range
	);
}

void BankResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 5);
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"volume", m_volume);
	s >> Member< float >(L"presence", m_presence);
	s >> Member< float >(L"presenceRate", m_presenceRate);
	s >> Member< float >(L"range", m_range);
}

	}
}
