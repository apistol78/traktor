#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/CoreClassFactory.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IEncoding.h"
#include "Core/Io/IStream.h"
#include "Core/Io/IVolume.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Io/Utf16Encoding.h"
#include "Core/Io/Utf32Encoding.h"
#include "Core/Io/Path.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyArray.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/ISharedMemory.h"
#include "Core/System/OS.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace
	{

RefArray< File > IVolume_find(IVolume* self, const std::wstring& mask)
{
	RefArray< File > files;
	self->find(mask, files);
	return files;
}

FileSystem* FileSystem_getInstance()
{
	return &FileSystem::getInstance();
}

RefArray< File > FileSystem_find(FileSystem* self, const std::wstring& mask)
{
	RefArray< File > files;
	self->find(mask, files);
	return files;
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

std::wstring StringReader_readChar(StringReader* self)
{
	wchar_t tmp[2] = { self->readChar(), 0 };
	return tmp;
}

Any StringReader_readLine(StringReader* self)
{
	std::wstring tmp;
	if (self->readLine(tmp) >= 0)
		return Any::fromString(tmp);
	else
		return Any();
}

std::wstring Environment_get(Environment* self, const std::wstring& name)
{
	return self->get(name);
}

Ref< traktor::IStream > IProcess_getStdOut(IProcess* self)
{
	return self->getPipeStream(IProcess::SpStdOut);
}

Ref< traktor::IStream > IProcess_getStdErr(IProcess* self)
{
	return self->getPipeStream(IProcess::SpStdErr);
}

bool IProcess_signalCtrlC(IProcess* self)
{
	return self->signal(IProcess::StCtrlC);
}

bool IProcess_signalCtrlBreak(IProcess* self)
{
	return self->signal(IProcess::StCtrlBreak);
}

OS* OS_getInstance()
{
	return &OS::getInstance();
}

Ref< Environment > OS_getEnvironment_0(OS* self)
{
	return self->getEnvironment();
}

std::wstring OS_getEnvironment_1(OS* self, const std::wstring& key)
{
	std::wstring value;
	if (self->getEnvironment(key, value))
		return value;
	else
		return L"";
}

Ref< IProcess > OS_execute(OS* self, const std::wstring& commandLine, const std::wstring& workingDirectory, const Environment* environment, bool redirect, bool mute, bool detach)
{
	return self->execute(commandLine, workingDirectory, environment, redirect, mute, detach);
}

#if defined(_WIN32)
Any OS_getRegistry(OS* self, const std::wstring& key, const std::wstring& subKey, const std::wstring& valueName)
{
	std::wstring value;
	if (self->getRegistry(key, subKey, valueName, value))
		return Any::fromString(value);
	else
		return Any();
}
#endif

IPropertyValue* PropertyArray_getProperty(PropertyArray* self, uint32_t index)
{
	return self->getProperty(index);
}

void PropertyGroup_setProperty(PropertyGroup* self, const std::wstring& propertyName, const Any& value)
{
	if (value.isBoolean())
		self->setProperty< PropertyBoolean >(propertyName, value.getBoolean());
	else if (value.isInteger())
		self->setProperty< PropertyInteger >(propertyName, value.getInteger());
	else if (value.isFloat())
		self->setProperty< PropertyFloat >(propertyName, value.getFloat());
	else if (value.isString())
		self->setProperty< PropertyString >(propertyName, value.getWideString());
	else if (is_a< IPropertyValue >(value.getObject()))
		self->setProperty(propertyName, static_cast< IPropertyValue* >(value.getObjectUnsafe()));
}

Any PropertyGroup_getProperty(PropertyGroup* self, const std::wstring& propertyName)
{
	const IPropertyValue* property = self->getProperty(propertyName);
	if (const PropertyBoolean* propertyBoolean = dynamic_type_cast< const PropertyBoolean* >(property))
		return Any::fromBoolean(*propertyBoolean);
	else if (const PropertyInteger* propertyInteger = dynamic_type_cast< const PropertyInteger* >(property))
		return Any::fromInteger(*propertyInteger);
	else if (const PropertyFloat* propertyFloat = dynamic_type_cast< const PropertyFloat* >(property))
		return Any::fromFloat(*propertyFloat);
	else if (const PropertyString* propertyString = dynamic_type_cast< const PropertyString* >(property))
		return Any::fromString(*propertyString);
	else
		return Any();
}

Any PropertyGroup_getPropertyRaw(PropertyGroup* self, const std::wstring& propertyName)
{
	IPropertyValue* property = self->getProperty(propertyName);
	if (property)
		return Any::fromObject(property);
	else
		return Any();
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.CoreClassFactory", 0, CoreClassFactory, IRuntimeClassFactory)

void CoreClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< DateTime > > classDateTime = new AutoRuntimeClass< DateTime >();
	classDateTime->addConstructor();
	classDateTime->addConstructor< uint64_t >();
	classDateTime->addConstructor< uint16_t, uint8_t, uint16_t, uint8_t, uint8_t, uint8_t >();
	classDateTime->addStaticMethod("now", &DateTime::now);
	classDateTime->addStaticMethod("parse", &DateTime::parse);
	classDateTime->addMethod("getYear", &DateTime::getYear);
	classDateTime->addMethod("getMonth", &DateTime::getMonth);
	classDateTime->addMethod("getDay", &DateTime::getDay);
	classDateTime->addMethod("getWeekDay", &DateTime::getWeekDay);
	classDateTime->addMethod("getYearDay", &DateTime::getYearDay);
	classDateTime->addMethod("isLeapYear", &DateTime::isLeapYear);
	classDateTime->addMethod("getHour", &DateTime::getHour);
	classDateTime->addMethod("getMinute", &DateTime::getMinute);
	classDateTime->addMethod("getSecond", &DateTime::getSecond);
	classDateTime->addMethod("getSecondsSinceEpoch", &DateTime::getSecondsSinceEpoch);
	classDateTime->addMethod("format", &DateTime::format);
	registrar->registerClass(classDateTime);

	Ref< AutoRuntimeClass< Path > > classPath = new AutoRuntimeClass< Path >();
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
	registrar->registerClass(classPath);

	Ref< AutoRuntimeClass< File > > classFile = new AutoRuntimeClass< File >();
	classFile->addConstructor();
	classFile->addConstructor< const Path&, uint64_t, uint32_t, const DateTime&, const DateTime&, const DateTime& >();
	classFile->addConstructor< const Path&, uint64_t, uint32_t >();
	classFile->addMethod("getPath", &File::getPath);
	classFile->addMethod("getSize", &File::getSize);
	classFile->addMethod("getFlags", &File::getFlags);
	classFile->addMethod("isNormal", &File::isNormal);
	classFile->addMethod("isReadOnly", &File::isReadOnly);
	classFile->addMethod("isHidden", &File::isHidden);
	classFile->addMethod("isArchive", &File::isArchive);
	classFile->addMethod("isDirectory", &File::isDirectory);
	classFile->addMethod("getCreationTime", &File::getCreationTime);
	classFile->addMethod("getLastAccessTime", &File::getLastAccessTime);
	classFile->addMethod("getLastWriteTime", &File::getLastWriteTime);
	registrar->registerClass(classFile);

	Ref< AutoRuntimeClass< IVolume > > classIVolume = new AutoRuntimeClass< IVolume >();
	classIVolume->addMethod("getDescription", &IVolume::getDescription);
	classIVolume->addMethod("get", &IVolume::get);
	classIVolume->addMethod("find", &IVolume_find);
	classIVolume->addMethod("modify", &IVolume::modify);
	classIVolume->addMethod("open", &IVolume::open);
	classIVolume->addMethod("exist", &IVolume::exist);
	classIVolume->addMethod("remove", &IVolume::remove);
	classIVolume->addMethod("move", &IVolume::move);
	classIVolume->addMethod("copy", &IVolume::copy);
	classIVolume->addMethod("makeDirectory", &IVolume::makeDirectory);
	classIVolume->addMethod("renameDirectory", &IVolume::renameDirectory);
	classIVolume->addMethod("setCurrentDirectory", &IVolume::setCurrentDirectory);
	classIVolume->addMethod("getCurrentDirectory", &IVolume::getCurrentDirectory);
	registrar->registerClass(classIVolume);

	Ref< AutoRuntimeClass< IStream > > classIStream = new AutoRuntimeClass< IStream >();
	classIStream->addMethod("close", &IStream::close);
	classIStream->addMethod("canRead", &IStream::canRead);
	classIStream->addMethod("canWrite", &IStream::canWrite);
	classIStream->addMethod("canSeek", &IStream::canSeek);
	classIStream->addMethod("tell", &IStream::tell);
	classIStream->addMethod("available", &IStream::available);
	classIStream->addMethod("flush", &IStream::flush);
	registrar->registerClass(classIStream);

	Ref< AutoRuntimeClass< BufferedStream > > classBufferedStream = new AutoRuntimeClass< BufferedStream >();
	classBufferedStream->addConstructor< IStream* >();
	classBufferedStream->addConstructor< IStream*, uint32_t >();
	registrar->registerClass(classBufferedStream);

	Ref< AutoRuntimeClass< FileSystem > > classFileSystem = new AutoRuntimeClass< FileSystem >();
	classFileSystem->addStaticMethod("getInstance", &FileSystem_getInstance);
	classFileSystem->addMethod("mount", &FileSystem::mount);
	classFileSystem->addMethod("umount", &FileSystem::umount);
	classFileSystem->addMethod("getVolumeCount", &FileSystem::getVolumeCount);
	//classFileSystem->addMethod("getVolume", &FileSystem::getVolume);
	classFileSystem->addMethod("getVolumeId", &FileSystem::getVolumeId);
	classFileSystem->addMethod("setCurrentVolume", &FileSystem::setCurrentVolume);
	classFileSystem->addMethod("getCurrentVolume", &FileSystem::getCurrentVolume);
	classFileSystem->addMethod("setCurrentVolumeAndDirectory", &FileSystem::setCurrentVolumeAndDirectory);
	classFileSystem->addMethod("get", &FileSystem::get);
	classFileSystem->addMethod("find", &FileSystem_find);
	classFileSystem->addMethod("modify", &FileSystem::modify);
	classFileSystem->addMethod("open", &FileSystem::open);
	classFileSystem->addMethod("exist", &FileSystem::exist);
	classFileSystem->addMethod("remove", &FileSystem::remove);
	classFileSystem->addMethod("move", &FileSystem::move);
	classFileSystem->addMethod("copy", &FileSystem::copy);
	classFileSystem->addMethod("makeDirectory", &FileSystem::makeDirectory);
	classFileSystem->addMethod("makeAllDirectories", &FileSystem::makeAllDirectories);
	classFileSystem->addMethod("removeDirectory", &FileSystem::removeDirectory);
	classFileSystem->addMethod("renameDirectory", &FileSystem::renameDirectory);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_1);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_2);
	classFileSystem->addMethod("getRelativePath", &FileSystem_getRelativePath);
	registrar->registerClass(classFileSystem);

	Ref< AutoRuntimeClass< IEncoding > > classIEncoding = new AutoRuntimeClass< IEncoding >();
	registrar->registerClass(classIEncoding);

	Ref< AutoRuntimeClass< AnsiEncoding > > classAnsiEncoding = new AutoRuntimeClass< AnsiEncoding >();
	classAnsiEncoding->addConstructor();
	registrar->registerClass(classAnsiEncoding);

	Ref< AutoRuntimeClass< Utf8Encoding > > classUtf8Encoding = new AutoRuntimeClass< Utf8Encoding >();
	classUtf8Encoding->addConstructor();
	registrar->registerClass(classUtf8Encoding);

	Ref< AutoRuntimeClass< Utf16Encoding > > classUtf16Encoding = new AutoRuntimeClass< Utf16Encoding >();
	classUtf16Encoding->addConstructor();
	registrar->registerClass(classUtf16Encoding);

	Ref< AutoRuntimeClass< Utf32Encoding > > classUtf32Encoding = new AutoRuntimeClass< Utf32Encoding >();
	classUtf32Encoding->addConstructor();
	registrar->registerClass(classUtf32Encoding);

	Ref< AutoRuntimeClass< StringReader > > classStringReader = new AutoRuntimeClass< StringReader >();
	classStringReader->addConstructor< traktor::IStream*, IEncoding* >();
	classStringReader->addMethod("readChar", &StringReader_readChar);
	classStringReader->addMethod("readLine", &StringReader_readLine);
	registrar->registerClass(classStringReader);

	Ref< AutoRuntimeClass< StreamCopy > > classStreamCopy = new AutoRuntimeClass< StreamCopy >();
	classStreamCopy->addConstructor< traktor::IStream*, traktor::IStream* >();
	classStreamCopy->addMethod("execute", &StreamCopy::execute);
	registrar->registerClass(classStreamCopy);

	Ref< AutoRuntimeClass< Environment > > classEnvironment = new AutoRuntimeClass< Environment >();
	classEnvironment->addConstructor();
	classEnvironment->addMethod("set", &Environment::set);
	classEnvironment->addMethod("has", &Environment::has);
	classEnvironment->addMethod("get", &Environment_get);
	registrar->registerClass(classEnvironment);

	Ref< AutoRuntimeClass< IProcess > > classIProcess = new AutoRuntimeClass< IProcess >();
	classIProcess->addMethod("getStdOut", &IProcess_getStdOut);
	classIProcess->addMethod("getStdErr", &IProcess_getStdErr);
	classIProcess->addMethod("signalCtrlC", &IProcess_signalCtrlC);
	classIProcess->addMethod("signalCtrlBreak", &IProcess_signalCtrlBreak);
	classIProcess->addMethod("exitCode", &IProcess::exitCode);
	registrar->registerClass(classIProcess);

	Ref< AutoRuntimeClass< ISharedMemory > > classISharedMemory = new AutoRuntimeClass< ISharedMemory >();
	//classISharedMemory->addMethod("read", &ISharedMemory::read);
	//classISharedMemory->addMethod("write", &ISharedMemory::write);
	classISharedMemory->addMethod("clear", &ISharedMemory::clear);
	registrar->registerClass(classISharedMemory);

	Ref< AutoRuntimeClass< OS > > classOS = new AutoRuntimeClass< OS >();
	classOS->addStaticMethod("getInstance", &OS_getInstance);
	classOS->addMethod("getCPUCoreCount", &OS::getCPUCoreCount);
	classOS->addMethod("getExecutable", &OS::getExecutable);
	classOS->addMethod("getCommandLine", &OS::getCommandLine);
	classOS->addMethod("getComputerName", &OS::getComputerName);
	classOS->addMethod("getCurrentUser", &OS::getCurrentUser);
	classOS->addMethod("getUserHomePath", &OS::getUserHomePath);
	classOS->addMethod("getUserApplicationDataPath", &OS::getUserApplicationDataPath);
	classOS->addMethod("getWritableFolderPath", &OS::getWritableFolderPath);
	classOS->addMethod("openFile", &OS::openFile);
	classOS->addMethod("editFile", &OS::editFile);
	classOS->addMethod("getEnvironment", &OS_getEnvironment_0);
	classOS->addMethod("getEnvironment", &OS_getEnvironment_1);
	classOS->addMethod("execute", &OS_execute);
	classOS->addMethod("createSharedMemory", &OS::createSharedMemory);
	classOS->addMethod("setOwnProcessPriorityBias", &OS::setOwnProcessPriorityBias);
#if defined(_WIN32)
	classOS->addMethod("getRegistry", &OS_getRegistry);
#endif
	registrar->registerClass(classOS);

	Ref< AutoRuntimeClass< DeepClone > > classDeepClone = new AutoRuntimeClass< DeepClone >();
	classDeepClone->addConstructor< const ISerializable* >();
	classDeepClone->addMethod< Ref< ISerializable > >("create", &DeepClone::create);
	registrar->registerClass(classDeepClone);

	Ref< AutoRuntimeClass< DeepHash > > classDeepHash = new AutoRuntimeClass< DeepHash >();
	classDeepHash->addConstructor< const ISerializable* >();
	classDeepHash->addMethod("get", &DeepHash::get);
	registrar->registerClass(classDeepHash);

	Ref< AutoRuntimeClass< IPropertyValue > > classIPropertyValue = new AutoRuntimeClass< IPropertyValue >();
	registrar->registerClass(classIPropertyValue);

	Ref< AutoRuntimeClass< PropertyArray > > classPropertyArray = new AutoRuntimeClass< PropertyArray >();
	classPropertyArray->addConstructor();
	classPropertyArray->addConstructor< const RefArray< IPropertyValue >& >();
	classPropertyArray->addMethod("addProperty", &PropertyArray::addProperty);
	classPropertyArray->addMethod("removeProperty", &PropertyArray::removeProperty);
	classPropertyArray->addMethod("getPropertyCount", &PropertyArray::getPropertyCount);
	classPropertyArray->addMethod("getProperty", &PropertyArray_getProperty);
	registrar->registerClass(classPropertyArray);

	Ref< AutoRuntimeClass< PropertyBoolean > > classPropertyBoolean = new AutoRuntimeClass< PropertyBoolean >();
	classPropertyBoolean->addConstructor();
	classPropertyBoolean->addConstructor< bool >();
	classPropertyBoolean->addMethod("get", &PropertyBoolean_get);
	registrar->registerClass(classPropertyBoolean);

	Ref< AutoRuntimeClass< PropertyColor > > classPropertyColor = new AutoRuntimeClass< PropertyColor >();
	classPropertyColor->addConstructor();
	registrar->registerClass(classPropertyColor);

	Ref< AutoRuntimeClass< PropertyFloat > > classPropertyFloat = new AutoRuntimeClass< PropertyFloat >();
	classPropertyFloat->addConstructor();
	classPropertyFloat->addConstructor< float >();
	classPropertyFloat->addMethod("get", &PropertyFloat_get);
	registrar->registerClass(classPropertyFloat);

	Ref< AutoRuntimeClass< PropertyGroup > > classPropertyGroup = new AutoRuntimeClass< PropertyGroup >();
	classPropertyGroup->addConstructor();
	classPropertyGroup->addMethod("setProperty", &PropertyGroup_setProperty);
	classPropertyGroup->addMethod("getProperty", &PropertyGroup_getProperty);
	classPropertyGroup->addMethod("getPropertyRaw", &PropertyGroup_getPropertyRaw);
	registrar->registerClass(classPropertyGroup);

	Ref< AutoRuntimeClass< PropertyInteger > > classPropertyInteger = new AutoRuntimeClass< PropertyInteger >();
	classPropertyInteger->addConstructor();
	classPropertyInteger->addConstructor< int32_t >();
	classPropertyInteger->addMethod("get", &PropertyInteger_get);
	registrar->registerClass(classPropertyInteger);

	Ref< AutoRuntimeClass< PropertyObject > > classPropertyObject = new AutoRuntimeClass< PropertyObject >();
	classPropertyObject->addConstructor();
	classPropertyObject->addMethod("get", &PropertyObject_get);
	registrar->registerClass(classPropertyObject);

	Ref< AutoRuntimeClass< PropertyString > > classPropertyString = new AutoRuntimeClass< PropertyString >();
	classPropertyString->addConstructor();
	classPropertyString->addConstructor< const std::wstring& >();
	classPropertyString->addMethod("get", &PropertyString_get);
	registrar->registerClass(classPropertyString);

	Ref< AutoRuntimeClass< PropertyStringArray > > classPropertyStringArray = new AutoRuntimeClass< PropertyStringArray >();
	classPropertyStringArray->addConstructor();
	registrar->registerClass(classPropertyStringArray);

	Ref< AutoRuntimeClass< PropertyStringSet > > classPropertyStringSet = new AutoRuntimeClass< PropertyStringSet >();
	classPropertyStringSet->addConstructor();
	registrar->registerClass(classPropertyStringSet);

	Ref< AutoRuntimeClass< Timer > > classTimer = new AutoRuntimeClass< Timer >();
	classTimer->addConstructor();
	classTimer->addMethod("start", &Timer::start);
	classTimer->addMethod("pause", &Timer::pause);
	classTimer->addMethod("stop", &Timer::stop);
	classTimer->addMethod("getElapsedTime", &Timer::getElapsedTime);
	classTimer->addMethod("getDeltaTime", &Timer::getDeltaTime);
	registrar->registerClass(classTimer);
}

}
