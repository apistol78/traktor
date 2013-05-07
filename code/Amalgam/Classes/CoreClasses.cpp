#include "Amalgam/Classes/CoreClasses.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

void PropertyGroup_setProperty(PropertyGroup* self, const std::wstring& propertyName, const script::Any& value)
{
	if (value.isBoolean())
		self->setProperty< PropertyBoolean >(propertyName, value.getBoolean());
	else if (value.isInteger())
		self->setProperty< PropertyInteger >(propertyName, value.getBoolean());
	else if (value.isFloat())
		self->setProperty< PropertyFloat >(propertyName, value.getBoolean());
	else if (value.isString())
		self->setProperty< PropertyString >(propertyName, value.getWideString());
}

script::Any PropertyGroup_getProperty(PropertyGroup* self, const std::wstring& propertyName)
{
	const IPropertyValue* property = self->getProperty(propertyName);
	if (const PropertyBoolean* propertyBoolean = dynamic_type_cast< const PropertyBoolean* >(property))
		return script::Any::fromBoolean(*propertyBoolean);
	else if (const PropertyInteger* propertyInteger = dynamic_type_cast< const PropertyInteger* >(property))
		return script::Any::fromInteger(*propertyInteger);
	else if (const PropertyFloat* propertyFloat = dynamic_type_cast< const PropertyFloat* >(property))
		return script::Any::fromFloat(*propertyFloat);
	else if (const PropertyString* propertyString = dynamic_type_cast< const PropertyString* >(property))
		return script::Any::fromString(*propertyString);
	else
		return script::Any();
}

		}

void registerCoreClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< IPropertyValue > > classIPropertyValue = new script::AutoScriptClass< IPropertyValue >();
	scriptManager->registerClass(classIPropertyValue);

	Ref< script::AutoScriptClass< PropertyBoolean > > classPropertyBoolean = new script::AutoScriptClass< PropertyBoolean >();
	classPropertyBoolean->addConstructor();
	classPropertyBoolean->addConstructor< bool >();
	scriptManager->registerClass(classPropertyBoolean);

	Ref< script::AutoScriptClass< PropertyColor > > classPropertyColor = new script::AutoScriptClass< PropertyColor >();
	classPropertyColor->addConstructor();
	scriptManager->registerClass(classPropertyColor);

	Ref< script::AutoScriptClass< PropertyFloat > > classPropertyFloat = new script::AutoScriptClass< PropertyFloat >();
	classPropertyFloat->addConstructor();
	classPropertyFloat->addConstructor< float >();
	scriptManager->registerClass(classPropertyFloat);

	Ref< script::AutoScriptClass< PropertyGroup > > classPropertyGroup = new script::AutoScriptClass< PropertyGroup >();
	classPropertyGroup->addConstructor();
	classPropertyGroup->addMethod("setProperty", &PropertyGroup_setProperty);
	classPropertyGroup->addMethod("getProperty", &PropertyGroup_getProperty);
	scriptManager->registerClass(classPropertyGroup);

	Ref< script::AutoScriptClass< PropertyInteger > > classPropertyInteger = new script::AutoScriptClass< PropertyInteger >();
	classPropertyInteger->addConstructor();
	classPropertyInteger->addConstructor< int32_t >();
	scriptManager->registerClass(classPropertyInteger);

	Ref< script::AutoScriptClass< PropertyObject > > classPropertyObject = new script::AutoScriptClass< PropertyObject >();
	classPropertyObject->addConstructor();
	scriptManager->registerClass(classPropertyObject);

	Ref< script::AutoScriptClass< PropertyString > > classPropertyString = new script::AutoScriptClass< PropertyString >();
	classPropertyString->addConstructor();
	classPropertyString->addConstructor< const std::wstring& >();
	scriptManager->registerClass(classPropertyString);

	Ref< script::AutoScriptClass< PropertyStringArray > > classPropertyStringArray = new script::AutoScriptClass< PropertyStringArray >();
	classPropertyStringArray->addConstructor();
	scriptManager->registerClass(classPropertyStringArray);

	Ref< script::AutoScriptClass< PropertyStringSet > > classPropertyStringSet = new script::AutoScriptClass< PropertyStringSet >();
	classPropertyStringSet->addConstructor();
	scriptManager->registerClass(classPropertyStringSet);
}

	}
}
