#include "Amalgam/Game/Engine/Action/AsSoundDriver.h"
#include "Sound/ISoundDriver.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.AsSoundDriver", AsSoundDriver, flash::ActionObjectRelay)

AsSoundDriver::AsSoundDriver()
:	flash::ActionObjectRelay("traktor.amalgam.SoundDriver")
{
}

AsSoundDriver::AsSoundDriver(const TypeInfo* soundDriverType)
:	flash::ActionObjectRelay("traktor.amalgam.SoundDriver")
,	m_soundDriverType(soundDriverType)
{
}

RefArray< AsSoundDriver > AsSoundDriver::getAvailable()
{
	std::set< const TypeInfo* > soundDriverTypes;
	type_of< sound::ISoundDriver >().findAllOf(soundDriverTypes, false);

	RefArray< AsSoundDriver > soundDrivers;
	for (std::set< const TypeInfo* >::const_iterator i = soundDriverTypes.begin(); i != soundDriverTypes.end(); ++i)
		soundDrivers.push_back(new AsSoundDriver(*i));

	return soundDrivers;
}

std::wstring AsSoundDriver::getName() const
{
	return m_soundDriverType ? m_soundDriverType->getName() : L"";
}

	}
}
