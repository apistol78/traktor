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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 1, BankResource, ISoundResource)

BankResource::BankResource()
{
}

BankResource::BankResource(const RefArray< IGrainData >& grains, float volume)
:	m_grains(grains)
,	m_volume(volume)
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

	return new Sound(new BankBuffer(grains), m_volume);
}

bool BankResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 1);
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
	s >> Member< float >(L"volume", m_volume);
	return true;
}

	}
}
