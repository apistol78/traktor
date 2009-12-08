#include "Core/Log/Log.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankResource", 0, BankResource, ISoundResource)

BankResource::BankResource()
{
}

BankResource::BankResource(const std::vector< resource::Proxy< Sound > >& sounds)
:	m_sounds(sounds)
{
}

Ref< Sound > BankResource::createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const
{
	for (std::vector< resource::Proxy< Sound > >::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		if (!resourceManager->bind(*i))
			return 0;
	}

	Ref< BankBuffer > bankBuffer = new BankBuffer(
		m_sounds
	);

	return new Sound(bankBuffer);
}

bool BankResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< resource::Proxy< Sound >, resource::Member< Sound, ISoundResource > >(L"sounds", m_sounds);
}

	}
}
