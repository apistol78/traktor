#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Parade/Classes/CoreClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{

void registerCoreClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< IPropertyValue > > classIPropertyValue = new script::AutoScriptClass< IPropertyValue >();
	scriptManager->registerClass(classIPropertyValue);

	Ref< script::AutoScriptClass< PropertyBoolean > > classPropertyBoolean = new script::AutoScriptClass< PropertyBoolean >();
	scriptManager->registerClass(classPropertyBoolean);

	Ref< script::AutoScriptClass< PropertyColor > > classPropertyColor = new script::AutoScriptClass< PropertyColor >();
	scriptManager->registerClass(classPropertyColor);

	Ref< script::AutoScriptClass< PropertyFloat > > classPropertyFloat = new script::AutoScriptClass< PropertyFloat >();
	scriptManager->registerClass(classPropertyFloat);

	Ref< script::AutoScriptClass< PropertyGroup > > classPropertyGroup = new script::AutoScriptClass< PropertyGroup >();
	scriptManager->registerClass(classPropertyGroup);

	Ref< script::AutoScriptClass< PropertyInteger > > classPropertyInteger = new script::AutoScriptClass< PropertyInteger >();
	scriptManager->registerClass(classPropertyInteger);

	Ref< script::AutoScriptClass< PropertyObject > > classPropertyObject = new script::AutoScriptClass< PropertyObject >();
	scriptManager->registerClass(classPropertyObject);

	Ref< script::AutoScriptClass< PropertyString > > classPropertyString = new script::AutoScriptClass< PropertyString >();
	scriptManager->registerClass(classPropertyString);

	Ref< script::AutoScriptClass< PropertyStringArray > > classPropertyStringArray = new script::AutoScriptClass< PropertyStringArray >();
	scriptManager->registerClass(classPropertyStringArray);

	Ref< script::AutoScriptClass< PropertyStringSet > > classPropertyStringSet = new script::AutoScriptClass< PropertyStringSet >();
	scriptManager->registerClass(classPropertyStringSet);
}

	}
}
