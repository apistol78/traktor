#include "Core/Log/Log.h"
#include "Flash/FlashButtonInstance.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsButton.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsButton", AsButton, ActionClass)

AsButton::AsButton(ActionContext* context)
:	ActionClass(context, "Button")
{
	Ref< ActionObject > prototype = new ActionObject();

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

void AsButton::init(ActionObject* self, const ActionValueArray& args) const
{
}

void AsButton::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_alpha(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_alpha(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_blendMode(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_blendMode(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_cacheAsBitmap(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_cacheAsBitmap(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_enabled(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_enabled(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_filters(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_filters(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_focusrect(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_focusrect(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_height(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_height(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_highquality(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_highquality(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_get_menu(const FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

void AsButton::Button_set_menu(FlashButtonInstance* self) const
{
	T_FATAL_ERROR;
}

std::string AsButton::Button_get_name(const FlashButtonInstance* self) const
{
	return self->getName();
}

void AsButton::Button_set_name(FlashButtonInstance* self, const std::string& name) const
{
	self->setName(name);
}

void AsButton::Button_get_parent(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_parent(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_quality(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_quality(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_rotation(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_rotation(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_scale9Grid(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_scale9Grid(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_soundbuftime(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_soundbuftime(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_tabEnabled(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_tabEnabled(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_tabIndex(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_tabIndex(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_target(const FlashButtonInstance* self) const
{
}

void AsButton::Button_get_trackAsMenu(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_trackAsMenu(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_url(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_url(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_useHandCursor(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_useHandCursor(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_visible(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_visible(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_width(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_width(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_x(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_x(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_xmouse(const FlashButtonInstance* self) const
{
}

void AsButton::Button_get_xscale(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_xscale(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_y(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_y(FlashButtonInstance* self) const
{
}

void AsButton::Button_get_ymouse(const FlashButtonInstance* self) const
{
}

void AsButton::Button_get_yscale(const FlashButtonInstance* self) const
{
}

void AsButton::Button_set_yscale(FlashButtonInstance* self) const
{
}

	}
}
