#include "Amalgam/Action/AsDisplayMode.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_DisplayMode.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_traktor_amalgam_DisplayMode", As_traktor_amalgam_DisplayMode, flash::ActionClass)

As_traktor_amalgam_DisplayMode::As_traktor_amalgam_DisplayMode(flash::ActionContext* context, amalgam::IEnvironment* environment)
:	flash::ActionClass(context, "traktor.amalgam.DisplayMode")
,	m_environment(environment)
{
	addProperty("availableModes", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_availableModes), 0);

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);

	prototype->addProperty("width", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_width), 0);
	prototype->addProperty("height", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_height), 0);
	prototype->addProperty("refreshRate", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_refreshRate), 0);
	prototype->addProperty("colorBits", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_colorBits), 0);
	prototype->addProperty("stereoscopic", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_stereoscopic), 0);

	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_amalgam_DisplayMode::initialize(flash::ActionObject* self)
{
}

void As_traktor_amalgam_DisplayMode::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	if (args.size() >= 4)
	{
		render::DisplayMode displayMode;
		displayMode.width = int32_t(args[0].getNumber());
		displayMode.height = int32_t(args[1].getNumber());
		displayMode.refreshRate = int32_t(args[2].getNumber());
		displayMode.colorBits = int32_t(args[3].getNumber());
		self->setRelay(new AsDisplayMode(
			displayMode
		));
	}
	else
		self->setRelay(new AsDisplayMode());
}

flash::ActionValue As_traktor_amalgam_DisplayMode::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_amalgam_DisplayMode::DisplayMode_get_availableModes(flash::CallArgs& ca)
{
	RefArray< AsDisplayMode > displayMode = AsDisplayMode::getAvailableModes(m_environment);
	
	Ref< flash::Array > displayModeArr = new flash::Array(displayMode.size());
	for (uint32_t i = 0; i < displayMode.size(); ++i)
	{
		displayModeArr->push(flash::ActionValue(
			displayMode[i]->getAsObject(getContext())
		));
	}

	ca.ret = flash::ActionValue(displayModeArr->getAsObject(getContext()));
}

uint32_t As_traktor_amalgam_DisplayMode::DisplayMode_get_width(const AsDisplayMode* self) const
{
	return self->getDisplayMode().width;
}

uint32_t As_traktor_amalgam_DisplayMode::DisplayMode_get_height(const AsDisplayMode* self) const
{
	return self->getDisplayMode().height;
}

uint32_t As_traktor_amalgam_DisplayMode::DisplayMode_get_refreshRate(const AsDisplayMode* self) const
{
	return self->getDisplayMode().refreshRate;
}

uint32_t As_traktor_amalgam_DisplayMode::DisplayMode_get_colorBits(const AsDisplayMode* self) const
{
	return self->getDisplayMode().colorBits;
}

bool As_traktor_amalgam_DisplayMode::DisplayMode_get_stereoscopic(const AsDisplayMode* self) const
{
	return self->getDisplayMode().stereoscopic;
}

	}
}
