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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 0, BankResource, ISoundResource)

BankResource::BankResource()
{
}

BankResource::BankResource(const RefArray< IGrainData >& grains)
:	m_grains(grains)
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

	return new Sound(new BankBuffer(grains));
}

bool BankResource::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
