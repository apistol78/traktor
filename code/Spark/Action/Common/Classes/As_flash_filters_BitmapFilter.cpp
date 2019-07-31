#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionFunctionNative.h"
#include "Spark/Action/Common/BitmapFilter.h"
#include "Spark/Action/Common/Classes/As_flash_filters_BitmapFilter.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.As_flash_filters_BitmapFilter", As_flash_filters_BitmapFilter, ActionClass)

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
