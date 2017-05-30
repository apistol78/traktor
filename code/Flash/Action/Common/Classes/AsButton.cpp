/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/ButtonInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsButton.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsButton", AsButton, ActionClass)

AsButton::AsButton(ActionContext* context)
:	ActionClass(context, "Button")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->addProperty("_alpha", createNativeFunction(context, this, &AsButton::Button_get_alpha), createNativeFunction(context, this, &AsButton::Button_set_alpha));
	prototype->addProperty("blendMode", createNativeFunction(context, this, &AsButton::Button_get_blendMode), createNativeFunction(context, this, &AsButton::Button_set_blendMode));
	prototype->addProperty("cacheAsBitmap", createNativeFunction(context, this, &AsButton::Button_get_cacheAsBitmap), createNativeFunction(context, this, &AsButton::Button_set_cacheAsBitmap));
	prototype->addProperty("enabled", createNativeFunction(context, this, &AsButton::Button_get_enabled), createNativeFunction(context, this, &AsButton::Button_set_enabled));
	prototype->addProperty("filters", createNativeFunction(context, this, &AsButton::Button_get_filters), createNativeFunction(context, this, &AsButton::Button_set_filters));
	prototype->addProperty("_focusrect", createNativeFunction(context, this, &AsButton::Button_get_focusrect), createNativeFunction(context, this, &AsButton::Button_set_focusrect));
	prototype->addProperty("_height", createNativeFunction(context, this, &AsButton::Button_get_height), createNativeFunction(context, this, &AsButton::Button_set_height));
	prototype->addProperty("_highquality", createNativeFunction(context, this, &AsButton::Button_get_highquality), createNativeFunction(context, this, &AsButton::Button_set_highquality));
	prototype->addProperty("menu", createNativeFunction(context, this, &AsButton::Button_get_menu), createNativeFunction(context, this, &AsButton::Button_set_menu));
	prototype->addProperty("_name", createNativeFunction(context, this, &AsButton::Button_get_name), createNativeFunction(context, this, &AsButton::Button_set_name));
	prototype->addProperty("_parent", createNativeFunction(context, this, &AsButton::Button_get_parent), createNativeFunction(context, this, &AsButton::Button_set_parent));
	prototype->addProperty("_quality", createNativeFunction(context, this, &AsButton::Button_get_quality), createNativeFunction(context, this, &AsButton::Button_set_quality));
	prototype->addProperty("_rotation", createNativeFunction(context, this, &AsButton::Button_get_rotation), createNativeFunction(context, this, &AsButton::Button_set_rotation));
	prototype->addProperty("scale9Grid", createNativeFunction(context, this, &AsButton::Button_get_scale9Grid), createNativeFunction(context, this, &AsButton::Button_set_scale9Grid));
	prototype->addProperty("_soundbuftime", createNativeFunction(context, this, &AsButton::Button_get_soundbuftime), createNativeFunction(context, this, &AsButton::Button_set_soundbuftime));
	prototype->addProperty("tabEnabled", createNativeFunction(context, this, &AsButton::Button_get_tabEnabled), createNativeFunction(context, this, &AsButton::Button_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(context, this, &AsButton::Button_get_tabIndex), createNativeFunction(context, this, &AsButton::Button_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(context, this, &AsButton::Button_get_target), 0);
	prototype->addProperty("trackAsMenu", createNativeFunction(context, this, &AsButton::Button_get_trackAsMenu), createNativeFunction(context, this, &AsButton::Button_set_trackAsMenu));
	prototype->addProperty("_url", createNativeFunction(context, this, &AsButton::Button_get_url), createNativeFunction(context, this, &AsButton::Button_set_url));
	prototype->addProperty("useHandCursor", createNativeFunction(context, this, &AsButton::Button_get_useHandCursor), createNativeFunction(context, this, &AsButton::Button_set_useHandCursor));
	prototype->addProperty("_visible", createNativeFunction(context, this, &AsButton::Button_get_visible), createNativeFunction(context, this, &AsButton::Button_set_visible));
	prototype->addProperty("_width", createNativeFunction(context, this, &AsButton::Button_get_width), createNativeFunction(context, this, &AsButton::Button_set_width));
	prototype->addProperty("_x", createNativeFunction(context, this, &AsButton::Button_get_x), createNativeFunction(context, this, &AsButton::Button_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(context, this, &AsButton::Button_get_xmouse), 0);
	prototype->addProperty("_xscale", createNativeFunction(context, this, &AsButton::Button_get_xscale), createNativeFunction(context, this, &AsButton::Button_set_xscale));
	prototype->addProperty("_y", createNativeFunction(context, this, &AsButton::Button_get_y), createNativeFunction(context, this, &AsButton::Button_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(context, this, &AsButton::Button_get_ymouse), 0);
	prototype->addProperty("_yscale", createNativeFunction(context, this, &AsButton::Button_get_yscale), createNativeFunction(context, this, &AsButton::Button_set_yscale));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsButton::initialize(ActionObject* self)
{
}

void AsButton::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsButton::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsButton::Button_get_alpha(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_alpha not implemented" << Endl;
	)
}

void AsButton::Button_set_alpha(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_alpha not implemented" << Endl;
	)
}

void AsButton::Button_get_blendMode(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_blendMode not implemented" << Endl;
	)
}

void AsButton::Button_set_blendMode(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_blendMode not implemented" << Endl;
	)
}

void AsButton::Button_get_cacheAsBitmap(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_cacheAsBitmap not implemented" << Endl;
	)
}

void AsButton::Button_set_cacheAsBitmap(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_cacheAsBitmap not implemented" << Endl;
	)
}

void AsButton::Button_get_enabled(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_enabled not implemented" << Endl;
	)
}

void AsButton::Button_set_enabled(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_enabled not implemented" << Endl;
	)
}

void AsButton::Button_get_filters(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_filters not implemented" << Endl;
	)
}

void AsButton::Button_set_filters(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_filters not implemented" << Endl;
	)
}

void AsButton::Button_get_focusrect(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_focusrect not implemented" << Endl;
	)
}

void AsButton::Button_set_focusrect(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_focusrect not implemented" << Endl;
	)
}

void AsButton::Button_get_height(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_height not implemented" << Endl;
	)
}

void AsButton::Button_set_height(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_height not implemented" << Endl;
	)
}

void AsButton::Button_get_highquality(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_highquality not implemented" << Endl;
	)
}

void AsButton::Button_set_highquality(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_highquality not implemented" << Endl;
	)
}

void AsButton::Button_get_menu(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_menu not implemented" << Endl;
	)
}

void AsButton::Button_set_menu(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_menu not implemented" << Endl;
	)
}

std::string AsButton::Button_get_name(const ButtonInstance* self) const
{
	return self->getName();
}

void AsButton::Button_set_name(ButtonInstance* self, const std::string& name) const
{
	self->setName(name);
}

void AsButton::Button_get_parent(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_parent not implemented" << Endl;
	)
}

void AsButton::Button_set_parent(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_parent not implemented" << Endl;
	)
}

void AsButton::Button_get_quality(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_quality not implemented" << Endl;
	)
}

void AsButton::Button_set_quality(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_quality not implemented" << Endl;
	)
}

void AsButton::Button_get_rotation(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_rotation not implemented" << Endl;
	)
}

void AsButton::Button_set_rotation(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_rotation not implemented" << Endl;
	)
}

void AsButton::Button_get_scale9Grid(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_scale9Grid not implemented" << Endl;
	)
}

void AsButton::Button_set_scale9Grid(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_scale9Grid not implemented" << Endl;
	)
}

void AsButton::Button_get_soundbuftime(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_soundbuftime not implemented" << Endl;
	)
}

void AsButton::Button_set_soundbuftime(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_soundbuftime not implemented" << Endl;
	)
}

void AsButton::Button_get_tabEnabled(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_tabEnabled not implemented" << Endl;
	)
}

void AsButton::Button_set_tabEnabled(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_tabEnabled not implemented" << Endl;
	)
}

void AsButton::Button_get_tabIndex(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_tabIndex not implemented" << Endl;
	)
}

void AsButton::Button_set_tabIndex(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_tabIndex not implemented" << Endl;
	)
}

void AsButton::Button_get_target(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_target not implemented" << Endl;
	)
}

void AsButton::Button_get_trackAsMenu(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_trackAsMenu not implemented" << Endl;
	)
}

void AsButton::Button_set_trackAsMenu(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_trackAsMenu not implemented" << Endl;
	)
}

void AsButton::Button_get_url(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_url not implemented" << Endl;
	)
}

void AsButton::Button_set_url(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_url not implemented" << Endl;
	)
}

void AsButton::Button_get_useHandCursor(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_useHandCursor not implemented" << Endl;
	)
}

void AsButton::Button_set_useHandCursor(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_useHandCursor not implemented" << Endl;
	)
}

void AsButton::Button_get_visible(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_visible not implemented" << Endl;
	)
}

void AsButton::Button_set_visible(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_visible not implemented" << Endl;
	)
}

void AsButton::Button_get_width(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_width not implemented" << Endl;
	)
}

void AsButton::Button_set_width(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_width not implemented" << Endl;
	)
}

void AsButton::Button_get_x(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_x not implemented" << Endl;
	)
}

void AsButton::Button_set_x(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_x not implemented" << Endl;
	)
}

void AsButton::Button_get_xmouse(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_xmouse not implemented" << Endl;
	)
}

void AsButton::Button_get_xscale(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_xscale not implemented" << Endl;
	)
}

void AsButton::Button_set_xscale(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_xscale not implemented" << Endl;
	)
}

void AsButton::Button_get_y(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_y not implemented" << Endl;
	)
}

void AsButton::Button_set_y(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_y not implemented" << Endl;
	)
}

void AsButton::Button_get_ymouse(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_ymouse not implemented" << Endl;
	)
}

void AsButton::Button_get_yscale(const ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::get_yscale not implemented" << Endl;
	)
}

void AsButton::Button_set_yscale(ButtonInstance* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Button::set_yscale not implemented" << Endl;
	)
}

	}
}
