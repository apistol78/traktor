/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/CoreClassFactory1.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/Boxes/BoxedColor4ub.h"
#include "Core/Class/Boxes/BoxedPointer.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Io/Utf16Encoding.h"
#include "Core/Io/Utf32Encoding.h"
#include "Core/Log/Log.h"
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

namespace traktor
{
	namespace
	{

float Reader_readFloat(Reader* self)
{
	float v; *self >> v;
	return v;
}

Ref< Path > Path_concat(Path* self, Path* rh)
{
	return new Path(*self + *rh);
}

RefArray< File > IVolume_find(IVolume* self, const std::wstring& mask)
{
	return self->find(mask);
}

bool IVolume_modify_1(IVolume* self, const Path& fileName, uint32_t flags)
{
	return self->modify(fileName, flags);
}

bool IVolume_modify_2(IVolume* self, const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime)
{
	return self->modify(fileName, creationTime, lastAccessTime, lastWriteTime);
}

uint32_t File_getSize(File* self)
{
	return uint32_t(self->getSize());
}

int64_t IStream_seek(IStream* self, int64_t origin, int64_t offset)
{
	return self->seek((IStream::SeekOriginType)origin, offset);
}

void FileSystem_setCurrentVolumeAndDirectory(FileSystem* self, const Path& directory)
{
	if (!self->setCurrentVolumeAndDirectory(directory))
		log::warning << L"Unable to set current volume and directory \"" << directory.getPathName() << L"\"." << Endl;
}

Path FileSystem_getCurrentVolumeAndDirectory(FileSystem* self)
{
	return self->getCurrentVolumeAndDirectory();
}

FileSystem* FileSystem_getInstance()
{
	return &FileSystem::getInstance();
}

IVolume* FileSystem_getVolume(FileSystem* self, int32_t index)
{
	return self->getVolume(index);
}

RefArray< File > FileSystem_find(FileSystem* self, const std::wstring& mask)
{
	return self->find(mask);
}

bool FileSystem_modify_1(FileSystem* self, const Path& fileName, uint32_t flags)
{
	return self->modify(fileName, flags);
}

bool FileSystem_modify_2(FileSystem* self, const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime)
{
	return self->modify(fileName, creationTime, lastAccessTime, lastWriteTime);
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
		return nullptr;
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

bool StreamCopy_execute(StreamCopy* self)
{
	return self->execute();
}

std::wstring Environment_get(Environment* self, const std::wstring& name)
{
	return self->get(name);
}

traktor::IStream* IProcess_getStdIn(IProcess* self)
{
	return self->getPipeStream(IProcess::SpStdIn);
}

traktor::IStream* IProcess_getStdOut(IProcess* self)
{
	return self->getPipeStream(IProcess::SpStdOut);
}

traktor::IStream* IProcess_getStdErr(IProcess* self)
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

bool IProcess_wait_0(IProcess* self)
{
	return self->wait();
}

bool IProcess_wait_1(IProcess* self, int32_t timeout)
{
	return self->wait(timeout);
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

Ref< IProcess > OS_execute(OS* self, const std::wstring& commandLine, const std::wstring& workingDirectory, const Environment* environment, uint32_t flags)
{
	return self->execute(commandLine, workingDirectory, environment, flags);
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
	else if (value.isInt32())
		self->setProperty< PropertyInteger >(propertyName, value.getInt32());
	else if (value.isInt64())
		self->setProperty< PropertyInteger >(propertyName, (int32_t)value.getInt64());
	else if (value.isFloat())
		self->setProperty< PropertyFloat >(propertyName, value.getFloat());
	else if (value.isString())
		self->setProperty< PropertyString >(propertyName, value.getWideString());
	else if (is_a< IPropertyValue >(value.getObject()))
		self->setProperty(propertyName, static_cast< IPropertyValue* >(value.getObjectUnsafe()));
}

Any PropertyGroup_getProperty(PropertyGroup* self, const std::wstring& propertyName)
{
	IPropertyValue* property = self->getProperty(propertyName);
	if (auto propertyBoolean = dynamic_type_cast< const PropertyBoolean* >(property))
		return Any::fromBoolean(*propertyBoolean);
	else if (auto propertyInteger = dynamic_type_cast< const PropertyInteger* >(property))
		return Any::fromInt32(*propertyInteger);
	else if (auto propertyFloat = dynamic_type_cast< const PropertyFloat* >(property))
		return Any::fromFloat(*propertyFloat);
	else if (auto propertyString = dynamic_type_cast< const PropertyString* >(property))
		return Any::fromString((const std::wstring&)*propertyString);
	else if (property)
		return Any::fromObject(property);
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

Color4ub PropertyColor_get(PropertyColor* self)
{
	return PropertyColor::get(self);
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.CoreClassFactory1", 0, CoreClassFactory1, IRuntimeClassFactory)

void CoreClassFactory1::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classDateTime = new AutoRuntimeClass< DateTime >();
	classDateTime->addConstructor();
	classDateTime->addConstructor< uint64_t >();
	classDateTime->addConstructor< uint16_t, uint8_t, uint16_t, uint8_t, uint8_t, uint8_t >();
	classDateTime->addProperty("year", &DateTime::getYear);
	classDateTime->addProperty("month", &DateTime::getMonth);
	classDateTime->addProperty("day", &DateTime::getDay);
	classDateTime->addProperty("weekDay", &DateTime::getWeekDay);
	classDateTime->addProperty("yearDay", &DateTime::getYearDay);
	classDateTime->addProperty("leapYear", &DateTime::isLeapYear);
	classDateTime->addProperty("hour", &DateTime::getHour);
	classDateTime->addProperty("minute", &DateTime::getMinute);
	classDateTime->addProperty("second", &DateTime::getSecond);
	classDateTime->addProperty("secondsSinceEpoch", &DateTime::getSecondsSinceEpoch);
	classDateTime->addStaticMethod("now", &DateTime::now);
	classDateTime->addStaticMethod("parse", &DateTime::parse);
	classDateTime->addMethod("format", &DateTime::format);
	registrar->registerClass(classDateTime);

	auto classPath = new AutoRuntimeClass< Path >();
	classPath->addConstructor();
	classPath->addConstructor< const std::wstring& >();
	classPath->addProperty("original", &Path::getOriginal);
	classPath->addProperty("hasVolume", &Path::hasVolume);
	classPath->addProperty("volume", &Path::getVolume);
	classPath->addProperty("relative", &Path::isRelative);
	classPath->addProperty("fileName", &Path::getFileName);
	classPath->addProperty("fileNameNoExtension", &Path::getFileNameNoExtension);
	classPath->addProperty("pathOnly", &Path::getPathOnly);
	classPath->addProperty("pathOnlyNoVolume", &Path::getPathOnlyNoVolume);
	classPath->addProperty("pathOnlyOS", &Path::getPathOnlyOS);
	classPath->addProperty("pathName", &Path::getPathName);
	classPath->addProperty("pathNameNoExtension", &Path::getPathNameNoExtension);
	classPath->addProperty("pathNameNoVolume", &Path::getPathNameNoVolume);
	classPath->addProperty("pathNameOS", &Path::getPathNameOS);
	classPath->addProperty("extension", &Path::getExtension);
	classPath->addMethod("normalized", &Path::normalized);
	classPath->addMethod("concat", &Path_concat);
	registrar->registerClass(classPath);

	auto classFile = new AutoRuntimeClass< File >();
	classFile->addConstant("FfNormal", Any::fromInt32(File::FfNormal));
	classFile->addConstant("FfReadOnly", Any::fromInt32(File::FfReadOnly));
	classFile->addConstant("FfHidden", Any::fromInt32(File::FfHidden));
	classFile->addConstant("FfArchive", Any::fromInt32(File::FfArchive));
	classFile->addConstant("FfDirectory", Any::fromInt32(File::FfDirectory));
	classFile->addConstant("FfExecutable", Any::fromInt32(File::FfExecutable));
	classFile->addConstant("FmRead", Any::fromInt32(File::FmRead));
	classFile->addConstant("FmWrite", Any::fromInt32(File::FmWrite));
	classFile->addConstant("FmAppend", Any::fromInt32(File::FmAppend));
	classFile->addConstant("FmMapped", Any::fromInt32(File::FmMapped));
	classFile->addConstructor();
	classFile->addConstructor< const Path&, uint64_t, uint32_t, const DateTime&, const DateTime&, const DateTime& >();
	classFile->addConstructor< const Path&, uint64_t, uint32_t >();
	classFile->addProperty("path", &File::getPath);
	classFile->addProperty("size", &File_getSize);
	classFile->addProperty("flags", &File::getFlags);
	classFile->addProperty("normal", &File::isNormal);
	classFile->addProperty("readOnly", &File::isReadOnly);
	classFile->addProperty("hidden", &File::isHidden);
	classFile->addProperty("archive", &File::isArchive);
	classFile->addProperty("directory", &File::isDirectory);
	classFile->addProperty("executable", &File::isExecutable);
	classFile->addProperty("creationTime", &File::getCreationTime);
	classFile->addProperty("lastAccessTime", &File::getLastAccessTime);
	classFile->addProperty("lastWriteTime", &File::getLastWriteTime);
	registrar->registerClass(classFile);

	auto classIVolume = new AutoRuntimeClass< IVolume >();
	classIVolume->addProperty("description", &IVolume::getDescription);
	classIVolume->addMethod("get", &IVolume::get);
	classIVolume->addMethod("find", &IVolume_find);
	classIVolume->addMethod("modify", &IVolume_modify_1);
	classIVolume->addMethod("modify", &IVolume_modify_2);
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

	auto classIStream = new AutoRuntimeClass< IStream >();
	classIStream->addConstant("SeekCurrent", Any::fromInt32(IStream::SeekCurrent));
	classIStream->addConstant("SeekEnd", Any::fromInt32(IStream::SeekEnd));
	classIStream->addConstant("SeekSet", Any::fromInt32(IStream::SeekSet));
	classIStream->addMethod("close", &IStream::close);
	classIStream->addMethod("canRead", &IStream::canRead);
	classIStream->addMethod("canWrite", &IStream::canWrite);
	classIStream->addMethod("canSeek", &IStream::canSeek);
	classIStream->addMethod("tell", &IStream::tell);
	classIStream->addMethod("available", &IStream::available);
	classIStream->addMethod("seek", &IStream_seek);
	classIStream->addMethod("flush", &IStream::flush);
	registrar->registerClass(classIStream);

	auto classBufferedStream = new AutoRuntimeClass< BufferedStream >();
	classBufferedStream->addConstructor< IStream* >();
	classBufferedStream->addConstructor< IStream*, uint32_t >();
	registrar->registerClass(classBufferedStream);

	auto classDynamicMemoryStream = new AutoRuntimeClass< DynamicMemoryStream >();
	classDynamicMemoryStream->addConstructor< bool, bool >();
	registrar->registerClass(classDynamicMemoryStream);

	auto classMemoryStream = new AutoRuntimeClass< MemoryStream >();
	classMemoryStream->addConstructor< void*, int64_t, bool, bool, bool >();
	classMemoryStream->addConstructor< const void*, int64_t >();
	registrar->registerClass(classMemoryStream);

	auto classBitReader = new AutoRuntimeClass< BitReader >();
	classBitReader->addConstructor< IStream* >();
	classBitReader->addProperty("stream", &BitReader::getStream);
	classBitReader->addMethod("readBit", &BitReader::readBit);
	classBitReader->addMethod("readUnsigned", &BitReader::readUnsigned);
	classBitReader->addMethod("readSigned", &BitReader::readSigned);
	classBitReader->addMethod("readInt8", &BitReader::readInt8);
	classBitReader->addMethod("readUInt8", &BitReader::readUInt8);
	classBitReader->addMethod("readInt16", &BitReader::readInt16);
	classBitReader->addMethod("readUInt16", &BitReader::readUInt16);
	classBitReader->addMethod("readInt32", &BitReader::readInt32);
	classBitReader->addMethod("readUInt32", &BitReader::readUInt32);
	classBitReader->addMethod("alignByte", &BitReader::alignByte);
	classBitReader->addMethod("tell", &BitReader::tell);
	classBitReader->addMethod("skip", &BitReader::skip);
	registrar->registerClass(classBitReader);

	auto classBitWriter = new AutoRuntimeClass< BitWriter >();
	classBitWriter->addConstructor< IStream* >();
	classBitWriter->addProperty("stream", &BitWriter::getStream);
	classBitWriter->addMethod("writeBit", &BitWriter::writeBit);
	classBitWriter->addMethod("writeUnsigned", &BitWriter::writeUnsigned);
	classBitWriter->addMethod("writeSigned", &BitWriter::writeSigned);
	classBitWriter->addMethod("writeInt8", &BitWriter::writeInt8);
	classBitWriter->addMethod("writeUInt8", &BitWriter::writeUInt8);
	classBitWriter->addMethod("writeInt16", &BitWriter::writeInt16);
	classBitWriter->addMethod("writeUInt16", &BitWriter::writeUInt16);
	classBitWriter->addMethod("writeInt32", &BitWriter::writeInt32);
	classBitWriter->addMethod("writeUInt32", &BitWriter::writeUInt32);
	classBitWriter->addMethod("writeFloat", &BitWriter::writeFloat);
	classBitWriter->addMethod("flush", &BitWriter::flush);
	classBitWriter->addMethod("tell", &BitWriter::tell);
	registrar->registerClass(classBitWriter);

	auto classReader = new AutoRuntimeClass< Reader >();
	classReader->addConstructor< IStream* >();
	classReader->addMethod("readFloat", &Reader_readFloat);
	registrar->registerClass(classReader);

	auto classFileSystem = new AutoRuntimeClass< FileSystem >();
	classFileSystem->addProperty("volumeCount", &FileSystem::getVolumeCount);
	classFileSystem->addProperty("currentVolume", &FileSystem::setCurrentVolume, &FileSystem::getCurrentVolume);
	classFileSystem->addProperty("currentVolumeAndDirectory", &FileSystem_setCurrentVolumeAndDirectory, &FileSystem_getCurrentVolumeAndDirectory);
	classFileSystem->addStaticMethod("getInstance", &FileSystem_getInstance);
	classFileSystem->addMethod("mount", &FileSystem::mount);
	classFileSystem->addMethod("umount", &FileSystem::umount);
	classFileSystem->addMethod("getVolume", &FileSystem_getVolume);
	classFileSystem->addMethod("getVolumeId", &FileSystem::getVolumeId);
	classFileSystem->addMethod("get", &FileSystem::get);
	classFileSystem->addMethod("find", &FileSystem_find);
	classFileSystem->addMethod("modify", &FileSystem_modify_1);
	classFileSystem->addMethod("modify", &FileSystem_modify_2);
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

	auto classIEncoding = new AutoRuntimeClass< IEncoding >();
	registrar->registerClass(classIEncoding);

	auto classAnsiEncoding = new AutoRuntimeClass< AnsiEncoding >();
	classAnsiEncoding->addConstructor();
	registrar->registerClass(classAnsiEncoding);

	auto classUtf8Encoding = new AutoRuntimeClass< Utf8Encoding >();
	classUtf8Encoding->addConstructor();
	registrar->registerClass(classUtf8Encoding);

	auto classUtf16Encoding = new AutoRuntimeClass< Utf16Encoding >();
	classUtf16Encoding->addConstructor();
	registrar->registerClass(classUtf16Encoding);

	auto classUtf32Encoding = new AutoRuntimeClass< Utf32Encoding >();
	classUtf32Encoding->addConstructor();
	registrar->registerClass(classUtf32Encoding);

	auto classStringReader = new AutoRuntimeClass< StringReader >();
	classStringReader->addConstructor< traktor::IStream*, IEncoding* >();
	classStringReader->addMethod("readChar", &StringReader_readChar);
	classStringReader->addMethod("readLine", &StringReader_readLine);
	registrar->registerClass(classStringReader);

	auto classStreamCopy = new AutoRuntimeClass< StreamCopy >();
	classStreamCopy->addConstructor< traktor::IStream*, traktor::IStream* >();
	classStreamCopy->addMethod("execute", &StreamCopy_execute);
	registrar->registerClass(classStreamCopy);

	auto classEnvironment = new AutoRuntimeClass< Environment >();
	classEnvironment->addConstructor();
	classEnvironment->addMethod("set", &Environment::set);
	classEnvironment->addMethod("has", &Environment::has);
	classEnvironment->addMethod("get", &Environment_get);
	registrar->registerClass(classEnvironment);

	auto classIProcess = new AutoRuntimeClass< IProcess >();
	classIProcess->addProperty("stdIn", &IProcess_getStdIn);
	classIProcess->addProperty("stdOut", &IProcess_getStdOut);
	classIProcess->addProperty("stdErr", &IProcess_getStdErr);
	classIProcess->addProperty("exitCode", &IProcess::exitCode);
	classIProcess->addMethod("signalCtrlC", &IProcess_signalCtrlC);
	classIProcess->addMethod("signalCtrlBreak", &IProcess_signalCtrlBreak);
	classIProcess->addMethod("wait", &IProcess_wait_0);
	classIProcess->addMethod("wait", &IProcess_wait_1);
	registrar->registerClass(classIProcess);

	auto classISharedMemory = new AutoRuntimeClass< ISharedMemory >();
	registrar->registerClass(classISharedMemory);

	auto classOS = new AutoRuntimeClass< OS >();
	classOS->addConstant("EfRedirectStdIO", Any::fromInt32(OS::EfRedirectStdIO));
	classOS->addConstant("EfMute", Any::fromInt32(OS::EfMute));
	classOS->addConstant("EfDetach", Any::fromInt32(OS::EfDetach));
	classOS->addStaticMethod("getInstance", &OS_getInstance);
	classOS->addProperty("name", &OS::getName);
	classOS->addProperty("identifier", &OS::getIdentifier);
	classOS->addProperty("cpuCoreCount", &OS::getCPUCoreCount);
	classOS->addProperty("executable", &OS::getExecutable);
	classOS->addProperty("commandLine", &OS::getCommandLine);
	classOS->addProperty("computerName", &OS::getComputerName);
	classOS->addProperty("currentUser", &OS::getCurrentUser);
	classOS->addProperty("userHomePath", &OS::getUserHomePath);
	classOS->addProperty("userApplicationDataPath", &OS::getUserApplicationDataPath);
	classOS->addProperty("writableFolderPath", &OS::getWritableFolderPath);
	classOS->addMethod("openFile", &OS::openFile);
	classOS->addMethod("editFile", &OS::editFile);
	classOS->addMethod("setEnvironment", &OS::setEnvironment);
	classOS->addMethod("getEnvironment", &OS_getEnvironment_0);
	classOS->addMethod("getEnvironment", &OS_getEnvironment_1);
	classOS->addMethod("execute", &OS_execute);
	classOS->addMethod("createSharedMemory", &OS::createSharedMemory);
	classOS->addMethod("setOwnProcessPriorityBias", &OS::setOwnProcessPriorityBias);
#if defined(_WIN32)
	classOS->addMethod("getRegistry", &OS_getRegistry);
#endif
	registrar->registerClass(classOS);

	auto classDeepClone = new AutoRuntimeClass< DeepClone >();
	classDeepClone->addConstructor< const ISerializable* >();
	classDeepClone->addMethod< Ref< ISerializable > >("create", &DeepClone::create);
	registrar->registerClass(classDeepClone);

	auto classDeepHash = new AutoRuntimeClass< DeepHash >();
	classDeepHash->addConstructor< const ISerializable* >();
	classDeepHash->addMethod("get", &DeepHash::get);
	registrar->registerClass(classDeepHash);

	auto classIPropertyValue = new AutoRuntimeClass< IPropertyValue >();
	registrar->registerClass(classIPropertyValue);

	auto classPropertyArray = new AutoRuntimeClass< PropertyArray >();
	classPropertyArray->addConstructor();
	classPropertyArray->addConstructor< const RefArray< IPropertyValue >& >();
	classPropertyArray->addMethod("addProperty", &PropertyArray::addProperty);
	classPropertyArray->addMethod("removeProperty", &PropertyArray::removeProperty);
	classPropertyArray->addMethod("getPropertyCount", &PropertyArray::getPropertyCount);
	classPropertyArray->addMethod("getProperty", &PropertyArray_getProperty);
	registrar->registerClass(classPropertyArray);

	auto classPropertyBoolean = new AutoRuntimeClass< PropertyBoolean >();
	classPropertyBoolean->addConstructor();
	classPropertyBoolean->addConstructor< bool >();
	classPropertyBoolean->addMethod("get", &PropertyBoolean_get);
	registrar->registerClass(classPropertyBoolean);

	auto classPropertyColor = new AutoRuntimeClass< PropertyColor >();
	classPropertyColor->addConstructor();
	classPropertyColor->addConstructor< Color4ub >();
	classPropertyColor->addMethod("get", &PropertyColor_get);
	registrar->registerClass(classPropertyColor);

	auto classPropertyFloat = new AutoRuntimeClass< PropertyFloat >();
	classPropertyFloat->addConstructor();
	classPropertyFloat->addConstructor< float >();
	classPropertyFloat->addMethod("get", &PropertyFloat_get);
	registrar->registerClass(classPropertyFloat);

	auto classPropertyGroup = new AutoRuntimeClass< PropertyGroup >();
	classPropertyGroup->addConstructor();
	classPropertyGroup->addMethod("setProperty", &PropertyGroup_setProperty);
	classPropertyGroup->addMethod("getProperty", &PropertyGroup_getProperty);
	classPropertyGroup->addMethod("getPropertyRaw", &PropertyGroup_getPropertyRaw);
	registrar->registerClass(classPropertyGroup);

	auto classPropertyInteger = new AutoRuntimeClass< PropertyInteger >();
	classPropertyInteger->addConstructor();
	classPropertyInteger->addConstructor< int32_t >();
	classPropertyInteger->addMethod("get", &PropertyInteger_get);
	registrar->registerClass(classPropertyInteger);

	auto classPropertyObject = new AutoRuntimeClass< PropertyObject >();
	classPropertyObject->addConstructor();
	classPropertyObject->addMethod("get", &PropertyObject_get);
	registrar->registerClass(classPropertyObject);

	auto classPropertyString = new AutoRuntimeClass< PropertyString >();
	classPropertyString->addConstructor();
	classPropertyString->addConstructor< const std::wstring& >();
	classPropertyString->addMethod("get", &PropertyString_get);
	registrar->registerClass(classPropertyString);

	auto classPropertyStringArray = new AutoRuntimeClass< PropertyStringArray >();
	classPropertyStringArray->addConstructor();
	registrar->registerClass(classPropertyStringArray);

	auto classPropertyStringSet = new AutoRuntimeClass< PropertyStringSet >();
	classPropertyStringSet->addConstructor();
	registrar->registerClass(classPropertyStringSet);
}

}
