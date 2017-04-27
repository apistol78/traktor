/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Matrix.h"
#include "Flash/Action/Common/Point.h"
#include "Flash/Action/Common/Classes/As_flash_geom_Matrix.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_geom_Matrix", As_flash_geom_Matrix, ActionClass)

As_flash_geom_Matrix::As_flash_geom_Matrix(ActionContext* context)
:	ActionClass(context, "flash.geom.Matrix")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("clone", ActionValue(createNativeFunction(context, &Matrix::clone)));
	prototype->setMember("concat", ActionValue(createNativeFunction(context, &Matrix::concat)));
	prototype->setMember("createBox", ActionValue(createNativeFunction(context, &Matrix::createBox)));
	prototype->setMember("createGradientBox", ActionValue(createNativeFunction(context, &Matrix::createGradientBox)));
	prototype->setMember("deltaTransformPoint", ActionValue(createNativeFunction(context, &Matrix::deltaTransformPoint)));
	prototype->setMember("identity", ActionValue(createNativeFunction(context, &Matrix::identity)));
	prototype->setMember("invert", ActionValue(createNativeFunction(context, &Matrix::invert)));
	prototype->setMember("rotate", ActionValue(createNativeFunction(context, &Matrix::rotate)));
	prototype->setMember("scale", ActionValue(createNativeFunction(context, &Matrix::scale)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, &Matrix::toString)));
	prototype->setMember("transformPoint", ActionValue(createNativeFunction(context, &Matrix::transformPoint)));
	prototype->setMember("translate", ActionValue(createNativeFunction(context, &Matrix::translate)));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_geom_Matrix::initialize(ActionObject* self)
{
}

void As_flash_geom_Matrix::construct(ActionObject* self, const ActionValueArray& args)
{
	float v[6] = { 0.0f };

	if (args.size() > 0)
		v[0] = args[0].getFloat();
	if (args.size() > 1)
		v[1] = args[1].getFloat();
	if (args.size() > 2)
		v[2] = args[2].getFloat();
	if (args.size() > 3)
		v[3] = args[3].getFloat();
	if (args.size() > 4)
		v[4] = args[4].getFloat();
	if (args.size() > 5)
		v[5] = args[5].getFloat();

	self->setRelay(new Matrix(v));
}

ActionValue As_flash_geom_Matrix::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
