#include "Amalgam/Engine/Classes/CoreClasses.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
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

FileSystem* FileSystem_getInstance()
{
	return &FileSystem::getInstance();
}

Path FileSystem_getAbsolutePath_1(FileSystem* self, const Path& relativePath)
{
	return self->getAbsolutePath(relativePath);
}

Path FileSystem_getAbsolutePath_2(FileSystem* self, const Path& basePath, const Path& relativePath)
{
	return self->getAbsolutePath(basePath, relativePath);
}

Ref< Path > FileSystem_getRelativePath(FileSystem* self, const Path& absolutePath, const Path& relativeToPath)
{
	Path relativePath;
	if (self->getRelativePath(absolutePath, relativeToPath, relativePath))
		return new Path(relativePath);
	else
		return 0;
}

void PropertyGroup_setProperty(PropertyGroup* self, const std::wstring& propertyName, const script::Any& value)
{
	if (value.isBoolean())
		self->setProperty< PropertyBoolean >(propertyName, value.getBoolean());
	else if (value.isInteger())
		self->setProperty< PropertyInteger >(propertyName, value.getInteger());
	else if (value.isFloat())
		self->setProperty< PropertyFloat >(propertyName, value.getFloat());
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

bool PropertyBoolean_get(PropertyBoolean* self)
{
	return PropertyBoolean::get(self);
}

float PropertyFloat_get(PropertyFloat* self)
{
	return PropertyFloat::get(self);
}

int32_t PropertyInteger_get(PropertyInteger* self)
{
	return PropertyInteger::get(self);
}

Object* PropertyObject_get(PropertyObject* self)
{
	return PropertyObject::get(self);
}

std::wstring PropertyString_get(PropertyString* self)
{
	return PropertyString::get(self);
}

		}

void registerCoreClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< Path > > classPath = new script::AutoScriptClass< Path >();
	classPath->addConstructor();
	classPath->addConstructor< const std::wstring& >();
	classPath->addMethod("getOriginal", &Path::getOriginal);
	classPath->addMethod("hasVolume", &Path::hasVolume);
	classPath->addMethod("getVolume", &Path::getVolume);
	classPath->addMethod("isRelative", &Path::isRelative);
	classPath->addMethod("getFileName", &Path::getFileName);
	classPath->addMethod("getFileNameNoExtension", &Path::getFileNameNoExtension);
	classPath->addMethod("getPathOnly", &Path::getPathOnly);
	classPath->addMethod("getPathOnlyNoVolume", &Path::getPathOnlyNoVolume);
	classPath->addMethod("getPathName", &Path::getPathName);
	classPath->addMethod("getPathNameNoExtension", &Path::getPathNameNoExtension);
	classPath->addMethod("getPathNameNoVolume", &Path::getPathNameNoVolume);
	classPath->addMethod("getExtension", &Path::getExtension);
	classPath->addMethod("normalized", &Path::normalized);
	scriptManager->registerClass(classPath);

	Ref< script::AutoScriptClass< IStream > > classIStream = new script::AutoScriptClass< IStream >();
	classIStream->addMethod("close", &IStream::close);
	classIStream->addMethod("canRead", &IStream::canRead);
	classIStream->addMethod("canWrite", &IStream::canWrite);
	classIStream->addMethod("canSeek", &IStream::canSeek);
	classIStream->addMethod("tell", &IStream::tell);
	classIStream->addMethod("available", &IStream::available);
	classIStream->addMethod("flush", &IStream::flush);
	scriptManager->registerClass(classIStream);

	Ref< script::AutoScriptClass< FileSystem > > classFileSystem = new script::AutoScriptClass< FileSystem >();
	classFileSystem->addStaticMethod("getInstance", &FileSystem_getInstance);
	classFileSystem->addMethod("open", &FileSystem::open);
	classFileSystem->addMethod("exist", &FileSystem::exist);
	classFileSystem->addMethod("remove", &FileSystem::remove);
	classFileSystem->addMethod("makeDirectory", &FileSystem::makeDirectory);
	classFileSystem->addMethod("makeAllDirectories", &FileSystem::makeAllDirectories);
	classFileSystem->addMethod("removeDirectory", &FileSystem::removeDirectory);
	classFileSystem->addMethod("renameDirectory", &FileSystem::renameDirectory);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_1);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_2);
	classFileSystem->addMethod("getRelativePath", &FileSystem_getRelativePath);
	scriptManager->registerClass(classFileSystem);

	Ref< script::AutoScriptClass< IPropertyValue > > classIPropertyValue = new script::AutoScriptClass< IPropertyValue >();
	scriptManager->registerClass(classIPropertyValue);

	Ref< script::AutoScriptClass< PropertyBoolean > > classPropertyBoolean = new script::AutoScriptClass< PropertyBoolean >();
	classPropertyBoolean->addConstructor();
	classPropertyBoolean->addConstructor< bool >();
	classPropertyBoolean->addMethod("get", &PropertyBoolean_get);
	scriptManager->registerClass(classPropertyBoolean);

	Ref< script::AutoScriptClass< PropertyColor > > classPropertyColor = new script::AutoScriptClass< PropertyColor >();
	classPropertyColor->addConstructor();
	scriptManager->registerClass(classPropertyColor);

	Ref< script::AutoScriptClass< PropertyFloat > > classPropertyFloat = new script::AutoScriptClass< PropertyFloat >();
	classPropertyFloat->addConstructor();
	classPropertyFloat->addConstructor< float >();
	classPropertyFloat->addMethod("get", &PropertyFloat_get);
	scriptManager->registerClass(classPropertyFloat);

	Ref< script::AutoScriptClass< PropertyGroup > > classPropertyGroup = new script::AutoScriptClass< PropertyGroup >();
	classPropertyGroup->addConstructor();
	classPropertyGroup->addMethod("setProperty", &PropertyGroup_setProperty);
	classPropertyGroup->addMethod("getProperty", &PropertyGroup_getProperty);
	scriptManager->registerClass(classPropertyGroup);

	Ref< script::AutoScriptClass< PropertyInteger > > classPropertyInteger = new script::AutoScriptClass< PropertyInteger >();
	classPropertyInteger->addConstructor();
	classPropertyInteger->addConstructor< int32_t >();
	classPropertyInteger->addMethod("get", &PropertyInteger_get);
	scriptManager->registerClass(classPropertyInteger);

	Ref< script::AutoScriptClass< PropertyObject > > classPropertyObject = new script::AutoScriptClass< PropertyObject >();
	classPropertyObject->addConstructor();
	classPropertyObject->addMethod("get", &PropertyObject_get);
	scriptManager->registerClass(classPropertyObject);

	Ref< script::AutoScriptClass< PropertyString > > classPropertyString = new script::AutoScriptClass< PropertyString >();
	classPropertyString->addConstructor();
	classPropertyString->addConstructor< const std::wstring& >();
	classPropertyString->addMethod("get", &PropertyString_get);
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
