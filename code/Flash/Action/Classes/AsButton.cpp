#include "Flash/Action/Classes/AsButton.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/FlashButtonInstance.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsButton", AsButton, ActionClass)

Ref< AsButton > AsButton::getInstance()
{
	static AsButton* instance = 0;
	if (!instance)
	{
		instance = new AsButton();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsButton::AsButton()
:	ActionClass(L"Button")
{
}

void AsButton::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));

	prototype->addProperty(L"_alpha", createNativeFunction(this, &AsButton::Button_get_alpha), createNativeFunction(this, &AsButton::Button_set_alpha));
	prototype->addProperty(L"blendMode", createNativeFunction(this, &AsButton::Button_get_blendMode), createNativeFunction(this, &AsButton::Button_set_blendMode));
	prototype->addProperty(L"cacheAsBitmap", createNativeFunction(this, &AsButton::Button_get_cacheAsBitmap), createNativeFunction(this, &AsButton::Button_set_cacheAsBitmap));
	prototype->addProperty(L"enabled", createNativeFunction(this, &AsButton::Button_get_enabled), createNativeFunction(this, &AsButton::Button_set_enabled));
	prototype->addProperty(L"filters", createNativeFunction(this, &AsButton::Button_get_filters), createNativeFunction(this, &AsButton::Button_set_filters));
	prototype->addProperty(L"_focusrect", createNativeFunction(this, &AsButton::Button_get_focusrect), createNativeFunction(this, &AsButton::Button_set_focusrect));
	prototype->addProperty(L"_height", createNativeFunction(this, &AsButton::Button_get_height), createNativeFunction(this, &AsButton::Button_set_height));
	prototype->addProperty(L"_highquality", createNativeFunction(this, &AsButton::Button_get_highquality), createNativeFunction(this, &AsButton::Button_set_highquality));
	prototype->addProperty(L"menu", createNativeFunction(this, &AsButton::Button_get_menu), createNativeFunction(this, &AsButton::Button_set_menu));
	prototype->addProperty(L"_name", createNativeFunction(this, &AsButton::Button_get_name), createNativeFunction(this, &AsButton::Button_set_name));
	prototype->addProperty(L"_parent", createNativeFunction(this, &AsButton::Button_get_parent), createNativeFunction(this, &AsButton::Button_set_parent));
	prototype->addProperty(L"_quality", createNativeFunction(this, &AsButton::Button_get_quality), createNativeFunction(this, &AsButton::Button_set_quality));
	prototype->addProperty(L"_rotation", createNativeFunction(this, &AsButton::Button_get_rotation), createNativeFunction(this, &AsButton::Button_set_rotation));
	prototype->addProperty(L"scale9Grid", createNativeFunction(this, &AsButton::Button_get_scale9Grid), createNativeFunction(this, &AsButton::Button_set_scale9Grid));
	prototype->addProperty(L"_soundbuftime", createNativeFunction(this, &AsButton::Button_get_soundbuftime), createNativeFunction(this, &AsButton::Button_set_soundbuftime));
	prototype->addProperty(L"tabEnabled", createNativeFunction(this, &AsButton::Button_get_tabEnabled), createNativeFunction(this, &AsButton::Button_set_tabEnabled));
	prototype->addProperty(L"tabIndex", createNativeFunction(this, &AsButton::Button_get_tabIndex), createNativeFunction(this, &AsButton::Button_set_tabIndex));
	prototype->addProperty(L"_target", createNativeFunction(this, &AsButton::Button_get_target), 0);
	prototype->addProperty(L"trackAsMenu", createNativeFunction(this, &AsButton::Button_get_trackAsMenu), createNativeFunction(this, &AsButton::Button_set_trackAsMenu));
	prototype->addProperty(L"_url", createNativeFunction(this, &AsButton::Button_get_url), createNativeFunction(this, &AsButton::Button_set_url));
	prototype->addProperty(L"useHandCursor", createNativeFunction(this, &AsButton::Button_get_useHandCursor), createNativeFunction(this, &AsButton::Button_set_useHandCursor));
	prototype->addProperty(L"_visible", createNativeFunction(this, &AsButton::Button_get_visible), createNativeFunction(this, &AsButton::Button_set_visible));
	prototype->addProperty(L"_width", createNativeFunction(this, &AsButton::Button_get_width), createNativeFunction(this, &AsButton::Button_set_width));
	prototype->addProperty(L"_x", createNativeFunction(this, &AsButton::Button_get_x), createNativeFunction(this, &AsButton::Button_set_x));
	prototype->addProperty(L"_xmouse", createNativeFunction(this, &AsButton::Button_get_xmouse), 0);
	prototype->addProperty(L"_xscale", createNativeFunction(this, &AsButton::Button_get_xscale), createNativeFunction(this, &AsButton::Button_set_xscale));
	prototype->addProperty(L"_y", createNativeFunction(this, &AsButton::Button_get_y), createNativeFunction(this, &AsButton::Button_set_y));
	prototype->addProperty(L"_ymouse", createNativeFunction(this, &AsButton::Button_get_ymouse), 0);
	prototype->addProperty(L"_yscale", createNativeFunction(this, &AsButton::Button_get_yscale), createNativeFunction(this, &AsButton::Button_set_yscale));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsButton::construct(ActionContext* context, const args_t& args)
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
