#include "Amalgam/Action/AsSoundDriver.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_SoundDriver.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_traktor_amalgam_SoundDriver", As_traktor_amalgam_SoundDriver, flash::ActionClass)

As_traktor_amalgam_SoundDriver::As_traktor_amalgam_SoundDriver(flash::ActionContext* context)
:	flash::ActionClass(context, "traktor.amalgam.SoundDriver")
{
	addProperty("available", createNativeFunction(context, this, &As_traktor_amalgam_SoundDriver::SoundDriver_get_available), 0);

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);

	prototype->addProperty("name", createNativeFunction(context, this, &As_traktor_amalgam_SoundDriver::SoundDriver_get_name), 0);

	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_amalgam_SoundDriver::initialize(flash::ActionObject* self)
{
}

void As_traktor_amalgam_SoundDriver::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	self->setRelay(new AsSoundDriver());
}

flash::ActionValue As_traktor_amalgam_SoundDriver::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_amalgam_SoundDriver::SoundDriver_get_available(flash::CallArgs& ca)
{
	RefArray< AsSoundDriver > soundDrivers = AsSoundDriver::getAvailable();
	
	Ref< flash::Array > soundDriversArr = new flash::Array(soundDrivers.size());
	for (uint32_t i = 0; i < soundDrivers.size(); ++i)
	{
		soundDriversArr->push(flash::ActionValue(
			soundDrivers[i]->getAsObject(getContext())
		));
	}

	ca.ret = flash::ActionValue(soundDriversArr->getAsObject(getContext()));
}

std::wstring As_traktor_amalgam_SoundDriver::SoundDriver_get_name(const AsSoundDriver* self) const
{
	return self->getName();
}

	}
}
