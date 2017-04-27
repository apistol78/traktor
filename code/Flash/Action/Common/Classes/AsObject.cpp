/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsObject", AsObject, ActionClass)

AsObject::AsObject(ActionContext* context)
:	ActionClass(context, "Object")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("addProperty", ActionValue(createNativeFunction(context, this, &AsObject::Object_addProperty)));
	prototype->setMember("hasOwnProperty", ActionValue(createNativeFunction(context, this, &AsObject::Object_hasOwnProperty)));
	prototype->setMember("isPropertyEnumerable", ActionValue(createNativeFunction(context, this, &AsObject::Object_isPropertyEnumerable)));
	prototype->setMember("isPrototypeOf", ActionValue(createNativeFunction(context, this, &AsObject::Object_isPrototypeOf)));
	prototype->setMember("registerClass", ActionValue(createNativeFunction(context, this, &AsObject::Object_registerClass)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsObject::Object_toString)));
	prototype->setMember("unwatch", ActionValue(createNativeFunction(context, this, &AsObject::Object_unwatch)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsObject::Object_valueOf)));
	prototype->setMember("watch", ActionValue(createNativeFunction(context, this, &AsObject::Object_watch)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsObject::initialize(ActionObject* self)
{
}

void AsObject::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsObject::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsObject::Object_addProperty(ActionObject* self, const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet) const
{
	self->addProperty(propertyName, propertyGet, propertySet);
}

bool AsObject::Object_hasOwnProperty(const ActionObject* self, const std::string& propertyName) const
{
	uint32_t propertyNameId = getContext()->getString(propertyName);
	return self->hasOwnMember(propertyNameId) || self->hasOwnProperty(propertyNameId);
}

bool AsObject::Object_isPropertyEnumerable(const ActionObject* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Object::isPropertyEnumerable not implemented" << Endl;
	)
	return true;
}

bool AsObject::Object_isPrototypeOf(const ActionObject* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Object::isPrototypeOf not implemented" << Endl;
	)
	return false;
}

void AsObject::Object_registerClass(CallArgs& ca)
{
	if (ca.args.size() < 1)
	{
		ca.ret = ActionValue(false);
		return;
	}

	Ref< ActionObject > global = ca.context->getGlobal();

	std::string movieClipName = ca.args[0].getString();
	uint32_t movieClipNameId = ca.context->getString(movieClipName);
	
	if (ca.args.size() >= 2)
	{
		ActionValue theClass = ca.args[1];
		if (theClass.isObject())
		{
			global->setMember(movieClipNameId, theClass);
			ca.ret = ActionValue(true);
		}
		else
			ca.ret = ActionValue(false);
	}
	else
		ca.ret = ActionValue(global->deleteMember(movieClipNameId));
}

void AsObject::Object_toString(CallArgs& ca)
{
	ca.ret = ActionValue("[object Object]");
}

void AsObject::Object_unwatch(ActionObject* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Object::unwatch not implemented" << Endl;
	)
}

void AsObject::Object_valueOf(CallArgs& ca)
{
	ca.ret = ActionValue(1);
}

void AsObject::Object_watch(ActionObject* self) const
{
	T_IF_VERBOSE(
		log::warning << L"Object::watch not implemented" << Endl;
	)
}

	}
}
