/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/Engine/Action/AsDisplayMode.h"
#include "Amalgam/Game/Engine/Action/Classes/As_traktor_amalgam_DisplayMode.h"
#include "Flash/Action/Common/Array.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_traktor_amalgam_DisplayMode", As_traktor_amalgam_DisplayMode, flash::ActionClass)

As_traktor_amalgam_DisplayMode::As_traktor_amalgam_DisplayMode(flash::ActionContext* context, IEnvironment* environment)
:	flash::ActionClass(context, "traktor.amalgam.DisplayMode")
,	m_environment(environment)
{
	addProperty("availableModes", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_availableModes), 0);
	addProperty("currentMode", createNativeFunction(context, this, &As_traktor_amalgam_DisplayMode::DisplayMode_get_currentMode), 0);

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
		displayMode.width = args[0].getInteger();
		displayMode.height = args[1].getInteger();
		displayMode.refreshRate = args[2].getInteger();
		displayMode.colorBits = args[3].getInteger();
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
	
	Ref< flash::Array > displayModeArr = new flash::Array(uint32_t(displayMode.size()));
	for (uint32_t i = 0; i < displayMode.size(); ++i)
	{
		displayModeArr->push(flash::ActionValue(
			displayMode[i]->getAsObject(getContext())
		));
	}

	ca.ret = flash::ActionValue(displayModeArr->getAsObject(getContext()));
}

void As_traktor_amalgam_DisplayMode::DisplayMode_get_currentMode(flash::CallArgs& ca)
{
	Ref< AsDisplayMode > displayMode = AsDisplayMode::getCurrentMode(m_environment);
	if (displayMode)
		ca.ret = flash::ActionValue(displayMode->getAsObject(getContext()));
	else
		ca.ret = flash::ActionValue();
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
