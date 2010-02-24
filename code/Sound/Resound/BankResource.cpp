#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 0, BankResource, ISoundResource)

BankResource::BankResource()
{
}

BankResource::BankResource(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

Ref< Sound > BankResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	for (RefArray< IGrain >::const_iterator i = m_grains.begin(); i != m_grains.end(); ++i)
	{
		if (!(*i)->bind(resourceManager))
			return 0;
	}

	Ref< BankBuffer > bankBuffer = new BankBuffer(m_grains);
	return new Sound(bankBuffer);
}

bool BankResource::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrain >(L"grains", m_grains);
}

	}
}
