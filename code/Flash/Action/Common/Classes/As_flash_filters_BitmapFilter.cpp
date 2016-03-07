#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/BitmapFilter.h"
#include "Flash/Action/Common/Classes/As_flash_filters_BitmapFilter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_filters_BitmapFilter", As_flash_filters_BitmapFilter, ActionClass)

As_flash_filters_BitmapFilter::As_flash_filters_BitmapFilter(ActionContext* context)
:	ActionClass(context, "flash.geom.BitmapFilter")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("clone", ActionValue(createNativeFunction(context, &BitmapFilter::clone)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_filters_BitmapFilter::initialize(ActionObject* self)
{
}

void As_flash_filters_BitmapFilter::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_filters_BitmapFilter::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
