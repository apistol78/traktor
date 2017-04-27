/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/BlurFilter.h"
#include "Flash/Action/Common/Classes/As_flash_filters_BlurFilter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_filters_BlurFilter", As_flash_filters_BlurFilter, ActionClass)

As_flash_filters_BlurFilter::As_flash_filters_BlurFilter(ActionContext* context)
:	ActionClass(context, "flash.filters.BlurFilter")
{
	Ref< ActionObject > prototype = new ActionObject(context, "flash.filters.BitmapFilter");

	prototype->addProperty("blurX", createNativeFunction(context, &BlurFilter::getBlurX), createNativeFunction(context, &BlurFilter::setBlurX));
	prototype->addProperty("blurY", createNativeFunction(context, &BlurFilter::getBlurY), createNativeFunction(context, &BlurFilter::setBlurY));
	prototype->addProperty("quality", createNativeFunction(context, &BlurFilter::getQuality), createNativeFunction(context, &BlurFilter::setQuality));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_filters_BlurFilter::initialize(ActionObject* self)
{
}

void As_flash_filters_BlurFilter::construct(ActionObject* self, const ActionValueArray& args)
{
	if (args.size() >= 3)
	{
		self->setRelay(new BlurFilter(
			args[0].getFloat(),
			args[1].getFloat(),
			args[2].getInteger()
		));
	}
	else
	{
		self->setRelay(new BlurFilter(4.0f, 4.0f, 1));
	}
}

ActionValue As_flash_filters_BlurFilter::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
