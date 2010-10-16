#include "Core/Log/Log.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsButton.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsButton", AsButton, ActionClass)

AsButton::AsButton()
:	ActionClass("Button")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->addProperty("_alpha", createNativeFunction(this, &AsButton::Button_get_alpha), createNativeFunction(this, &AsButton::Button_set_alpha));
	prototype->addProperty("blendMode", createNativeFunction(this, &AsButton::Button_get_blendMode), createNativeFunction(this, &AsButton::Button_set_blendMode));
	prototype->addProperty("cacheAsBitmap", createNativeFunction(this, &AsButton::Button_get_cacheAsBitmap), createNativeFunction(this, &AsButton::Button_set_cacheAsBitmap));
	prototype->addProperty("enabled", createNativeFunction(this, &AsButton::Button_get_enabled), createNativeFunction(this, &AsButton::Button_set_enabled));
	prototype->addProperty("filters", createNativeFunction(this, &AsButton::Button_get_filters), createNativeFunction(this, &AsButton::Button_set_filters));
	prototype->addProperty("_focusrect", createNativeFunction(this, &AsButton::Button_get_focusrect), createNativeFunction(this, &AsButton::Button_set_focusrect));
	prototype->addProperty("_height", createNativeFunction(this, &AsButton::Button_get_height), createNativeFunction(this, &AsButton::Button_set_height));
	prototype->addProperty("_highquality", createNativeFunction(this, &AsButton::Button_get_highquality), createNativeFunction(this, &AsButton::Button_set_highquality));
	prototype->addProperty("menu", createNativeFunction(this, &AsButton::Button_get_menu), createNativeFunction(this, &AsButton::Button_set_menu));
	prototype->addProperty("_name", createNativeFunction(this, &AsButton::Button_get_name), createNativeFunction(this, &AsButton::Button_set_name));
	prototype->addProperty("_parent", createNativeFunction(this, &AsButton::Button_get_parent), createNativeFunction(this, &AsButton::Button_set_parent));
	prototype->addProperty("_quality", createNativeFunction(this, &AsButton::Button_get_quality), createNativeFunction(this, &AsButton::Button_set_quality));
	prototype->addProperty("_rotation", createNativeFunction(this, &AsButton::Button_get_rotation), createNativeFunction(this, &AsButton::Button_set_rotation));
	prototype->addProperty("scale9Grid", createNativeFunction(this, &AsButton::Button_get_scale9Grid), createNativeFunction(this, &AsButton::Button_set_scale9Grid));
	prototype->addProperty("_soundbuftime", createNativeFunction(this, &AsButton::Button_get_soundbuftime), createNativeFunction(this, &AsButton::Button_set_soundbuftime));
	prototype->addProperty("tabEnabled", createNativeFunction(this, &AsButton::Button_get_tabEnabled), createNativeFunction(this, &AsButton::Button_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(this, &AsButton::Button_get_tabIndex), createNativeFunction(this, &AsButton::Button_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(this, &AsButton::Button_get_target), 0);
	prototype->addProperty("trackAsMenu", createNativeFunction(this, &AsButton::Button_get_trackAsMenu), createNativeFunction(this, &AsButton::Button_set_trackAsMenu));
	prototype->addProperty("_url", createNativeFunction(this, &AsButton::Button_get_url), createNativeFunction(this, &AsButton::Button_set_url));
	prototype->addProperty("useHandCursor", createNativeFunction(this, &AsButton::Button_get_useHandCursor), createNativeFunction(this, &AsButton::Button_set_useHandCursor));
	prototype->addProperty("_visible", createNativeFunction(this, &AsButton::Button_get_visible), createNativeFunction(this, &AsButton::Button_set_visible));
	prototype->addProperty("_width", createNativeFunction(this, &AsButton::Button_get_width), createNativeFunction(this, &AsButton::Button_set_width));
	prototype->addProperty("_x", createNativeFunction(this, &AsButton::Button_get_x), createNativeFunction(this, &AsButton::Button_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(this, &AsButton::Button_get_xmouse), 0);
	prototype->addProperty("_xscale", createNativeFunction(this, &AsButton::Button_get_xscale), createNativeFunction(this, &AsButton::Button_set_xscale));
	prototype->addProperty("_y", createNativeFunction(this, &AsButton::Button_get_y), createNativeFunction(this, &AsButton::Button_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(this, &AsButton::Button_get_ymouse), 0);
	prototype->addProperty("_yscale", createNativeFunction(this, &AsButton::Button_get_yscale), createNativeFunction(this, &AsButton::Button_set_yscale));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsButton::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsButton::Button_get_alpha(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_alpha(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_blendMode(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_blendMode(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_cacheAsBitmap(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_cacheAsBitmap(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_enabled(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_enabled(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_filters(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_filters(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_focusrect(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_focusrect(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_height(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_height(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_highquality(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_highquality(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_menu(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_menu(CallArgs& ca)
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_name(CallArgs& ca)
{
	FlashButtonInstance* buttonInstance = checked_type_cast< FlashButtonInstance* >(ca.self);
	ca.ret = ActionValue(buttonInstance->getName());
}

void AsButton::Button_set_name(CallArgs& ca)
{
	FlashButtonInstance* buttonInstance = checked_type_cast< FlashButtonInstance* >(ca.self);
	buttonInstance->setName(ca.args[0].getStringSafe());
}

void AsButton::Button_get_parent(CallArgs& ca)
{
}

void AsButton::Button_set_parent(CallArgs& ca)
{
}

void AsButton::Button_get_quality(CallArgs& ca)
{
}

void AsButton::Button_set_quality(CallArgs& ca)
{
}

void AsButton::Button_get_rotation(CallArgs& ca)
{
}

void AsButton::Button_set_rotation(CallArgs& ca)
{
}

void AsButton::Button_get_scale9Grid(CallArgs& ca)
{
}

void AsButton::Button_set_scale9Grid(CallArgs& ca)
{
}

void AsButton::Button_get_soundbuftime(CallArgs& ca)
{
}

void AsButton::Button_set_soundbuftime(CallArgs& ca)
{
}

void AsButton::Button_get_tabEnabled(CallArgs& ca)
{
}

void AsButton::Button_set_tabEnabled(CallArgs& ca)
{
}

void AsButton::Button_get_tabIndex(CallArgs& ca)
{
}

void AsButton::Button_set_tabIndex(CallArgs& ca)
{
}

void AsButton::Button_get_target(CallArgs& ca)
{
}

void AsButton::Button_get_trackAsMenu(CallArgs& ca)
{
}

void AsButton::Button_set_trackAsMenu(CallArgs& ca)
{
}

void AsButton::Button_get_url(CallArgs& ca)
{
}

void AsButton::Button_set_url(CallArgs& ca)
{
}

void AsButton::Button_get_useHandCursor(CallArgs& ca)
{
}

void AsButton::Button_set_useHandCursor(CallArgs& ca)
{
}

void AsButton::Button_get_visible(CallArgs& ca)
{
}

void AsButton::Button_set_visible(CallArgs& ca)
{
}

void AsButton::Button_get_width(CallArgs& ca)
{
}

void AsButton::Button_set_width(CallArgs& ca)
{
}

void AsButton::Button_get_x(CallArgs& ca)
{
}

void AsButton::Button_set_x(CallArgs& ca)
{
}

void AsButton::Button_get_xmouse(CallArgs& ca)
{
}

void AsButton::Button_get_xscale(CallArgs& ca)
{
}

void AsButton::Button_set_xscale(CallArgs& ca)
{
}

void AsButton::Button_get_y(CallArgs& ca)
{
}

void AsButton::Button_set_y(CallArgs& ca)
{
}

void AsButton::Button_get_ymouse(CallArgs& ca)
{
}

void AsButton::Button_get_yscale(CallArgs& ca)
{
}

void AsButton::Button_set_yscale(CallArgs& ca)
{
}

	}
}
