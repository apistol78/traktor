#include <Core/Date/DateTime.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Io/BufferedStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Path.h>
#include <Core/Io/StreamCopy.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Io/Utf16Encoding.h>
#include <Core/Io/Utf32Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/SafeDestroy.h>
#include <Core/Serialization/DeepClone.h>
#include <Core/Serialization/DeepHash.h>
#include <Core/System/IProcess.h>
#include <Core/System/ISharedMemory.h>
#include <Core/System/OS.h>
#include <Drawing/Image.h>
#include <Drawing/Filters/BrightnessContrastFilter.h>
#include <Drawing/Filters/ChainFilter.h>
#include <Drawing/Filters/ConvolutionFilter.h>
#include <Drawing/Filters/CropFilter.h>
#include <Drawing/Filters/DilateFilter.h>
#include <Drawing/Filters/GammaFilter.h>
#include <Drawing/Filters/GrayscaleFilter.h>
#include <Drawing/Filters/MirrorFilter.h>
#include <Drawing/Filters/NormalizeFilter.h>
#include <Drawing/Filters/NormalMapFilter.h>
#include <Drawing/Filters/PerlinNoiseFilter.h>
#include <Drawing/Filters/PremultiplyAlphaFilter.h>
#include <Drawing/Filters/QuantizeFilter.h>
#include <Drawing/Filters/ScaleFilter.h>
#include <Drawing/Filters/SharpenFilter.h>
#include <Drawing/Filters/SwizzleFilter.h>
#include <Drawing/Filters/TonemapFilter.h>
#include <Drawing/Filters/TransformFilter.h>
#include <Net/MulticastUdpSocket.h>
#include <Net/SocketAddressIPv4.h>
#include <Net/SocketAddressIPv6.h>
#include <Net/SocketSet.h>
#include <Net/TcpSocket.h>
#include <Net/UdpSocket.h>
#include <Net/UrlConnection.h>
#include <Net/Http/HttpRequest.h>
#include <Net/Http/HttpServer.h>
#include <Script/AutoScriptClass.h>
#include <Script/Boxes.h>
#include <Script/IScriptContext.h>
#include <Script/IScriptDelegate.h>
#include <Script/Lua/ScriptManagerLua.h>
#include <Sql/IResultSet.h>
#include <Sql/Sqlite3/ConnectionSqlite3.h>
#include <Xml/Attribute.h>
#include <Xml/Document.h>
#include <Xml/Element.h>
#include <Xml/Text.h>
#include "Environment.h"
#include "ProduceOutput.h"
#include "Run.h"
#include "StdOutput.h"
#include "StreamInput.h"
#include "StreamOutput.h"

using namespace traktor;

int32_t Run_run_1(Run* self, const std::wstring& command)
{
	return self->run(command);
}

int32_t Run_run_2(Run* self, const std::wstring& command, const std::wstring& saveOutputAs)
{
	return self->run(command, saveOutputAs);
}

int32_t Run_execute_1(Run* self, const std::wstring& command)
{
	return self->execute(command);
}

int32_t Run_execute_2(Run* self, const std::wstring& command, const std::wstring& saveOutputAs)
{
	return self->execute(command, saveOutputAs);
}

RefArray< File > IVolume_find(IVolume* self, const std::wstring& mask)
{
	RefArray< File > files;
	self->find(mask, files);
	return files;
}

RefArray< File > FileSystem_find(FileSystem* self, const std::wstring& mask)
{
	RefArray< File > files;
	self->find(mask, files);
	return files;
}

Path FileSystem_getAbsolutePath_1(FileSystem* fileSystem, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(relativePath);
}

Path FileSystem_getAbsolutePath_2(FileSystem* fileSystem, const Path& basePath, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(basePath, relativePath);
}

Ref< Path > FileSystem_getRelativePath(FileSystem* fileSystem, const Path& absolutePath, const Path& relativeToPath)
{
	Path relativePath;
	if (fileSystem->getRelativePath(absolutePath, relativeToPath, relativePath))
		return new Path(relativePath);
	else
		return 0;
}

std::wstring StringReader_readChar(StringReader* self)
{
	wchar_t tmp[2] = { self->readChar(), 0 };
	return tmp;
}

script::Any StringReader_readLine(StringReader* self)
{
	std::wstring tmp;
	if (self->readLine(tmp) >= 0)
		return script::Any::fromString(tmp);
	else
		return script::Any();
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

Ref< Environment > OS_getEnvironment_0(OS* self)
{
	return new Environment(self->getEnvironment());
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
	return self->execute(commandLine, workingDirectory, environment ? &environment->envmap() : 0, redirect, mute, detach);
}

Ref< drawing::CropFilter > drawing_CropFilter_constructor(int32_t anchorX, int32_t anchorY, int32_t width, int32_t height)
{
	return new drawing::CropFilter((drawing::CropFilter::AnchorType)anchorX, (drawing::CropFilter::AnchorType)anchorY, width, height);
}

Ref< drawing::ScaleFilter > drawing_ScaleFilter_constructor(int32_t width, int32_t height)
{
	return new drawing::ScaleFilter(width, height, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
}

Color4f drawing_Image_getPixel(drawing::Image* image, int32_t x, int32_t y)
{
	Color4f c;
	image->getPixel(x, y, c);
	return c;
}

Ref< drawing::Image > drawing_Image_loadFromFile(const std::wstring& filePath)
{
	return drawing::Image::load(filePath);
}

Ref< drawing::Image > drawing_Image_loadFromStream(traktor::IStream* stream, const std::wstring& extension)
{
	return drawing::Image::load(stream, extension);
}

bool drawing_Image_save(drawing::Image* image, const std::wstring& filePath)
{
	return image->save(filePath);
}

std::wstring xml_Node_write(xml::Node* node)
{
	StringOutputStream ss;
	node->write(ss);
	return ss.str();
}

RefArray< xml::Element > xml_Element_get(xml::Element* element, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	element->get(path, elements);
	return elements;
}

Ref< xml::Attribute > xml_Element_getAttribute_1(xml::Element* element, const std::wstring& name)
{
	return element->getAttribute(name);
}

Ref< xml::Attribute > xml_Element_getAttribute_2(xml::Element* element, const std::wstring& name, const std::wstring& defaultValue)
{
	return element->getAttribute(name, defaultValue);
}

RefArray< xml::Element > xml_Document_get(xml::Document* document, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	document->get(path, elements);
	return elements;
}

std::wstring net_HttpRequest_getMethod(net::HttpRequest* self)
{
	switch (self->getMethod())
	{
	case net::HttpRequest::MtGet:
		return L"GET";
	case net::HttpRequest::MtHead:
		return L"HEAD";
	case net::HttpRequest::MtPost:
		return L"POST";
	case net::HttpRequest::MtPut:
		return L"PUT";
	case net::HttpRequest::MtDelete:
		return L"DELETE";
	case net::HttpRequest::MtTrace:
		return L"TRACE";
	case net::HttpRequest::MtOptions:
		return L"OPTIONS";
	case net::HttpRequest::MtConnect:
		return L"CONNECT";
	case net::HttpRequest::MtPatch:
		return L"PATCH";
	default:
		return L"";
	}
}

bool net_HttpServer_create(net::HttpServer* self, int32_t port)
{
	return self->create(net::SocketAddressIPv4(port));
}

class HttpServerListenerDelegate : public net::HttpServer::IRequestListener
{
public:
	HttpServerListenerDelegate(script::IScriptDelegate* delegateListener)
	:	m_delegateListener(delegateListener)
	{
	}

	virtual int32_t httpClientRequest(net::HttpServer* server, const net::HttpRequest* request, OutputStream& os, Ref< traktor::IStream >& outStream)
	{
		script::Any argv[] =
		{
			script::CastAny< net::HttpServer* >::set(server),
			script::CastAny< const net::HttpRequest* >::set(request)
		};
		script::Any ret = m_delegateListener->call(sizeof_array(argv), argv);
		if (ret.isString())
		{
			os << ret.getWideString();
			return 200;
		}
		else
			return 404;
	}

private:
	Ref< script::IScriptDelegate > m_delegateListener;
};

void net_HttpServer_setRequestListener(net::HttpServer* self, script::IScriptDelegate* delegateListener)
{
	self->setRequestListener(new HttpServerListenerDelegate(delegateListener));
}

Ref< script::IScriptManager > createScriptManager()
{
	Ref< script::IScriptManager > scriptManager = new script::ScriptManagerLua();

	script::registerBoxClasses(scriptManager);

	//Ref< script::AutoScriptClass< Connection > > classConnection = new script::AutoScriptClass< Connection >();
	//classConnection->addStaticMethod("connect", &Connection::connect);
	//scriptManager->registerClass(classConnection);

	// IOutput
	Ref< script::AutoScriptClass< IOutput > > classIOutput = new script::AutoScriptClass< IOutput >();
	classIOutput->addMethod("print", &IOutput::print);
	classIOutput->addMethod("printLn", &IOutput::printLn);
	scriptManager->registerClass(classIOutput);

	// ProduceOutput
	Ref< script::AutoScriptClass< ProduceOutput > > classProduceOutput = new script::AutoScriptClass< ProduceOutput >();
	classProduceOutput->addMethod("printSection", &ProduceOutput::printSection);
	classProduceOutput->addMethod("addSection", &ProduceOutput::addSection);
	scriptManager->registerClass(classProduceOutput);

	// StdOutput
	Ref< script::AutoScriptClass< StdOutput > > classStdOutput = new script::AutoScriptClass< StdOutput >();
	scriptManager->registerClass(classStdOutput);

	// StreamOutput
	Ref< script::AutoScriptClass< StreamOutput > > classStreamOutput = new script::AutoScriptClass< StreamOutput >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding* >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding*, const std::wstring& >();
	scriptManager->registerClass(classStreamOutput);

	// IInput
	Ref< script::AutoScriptClass< IInput > > classIInput = new script::AutoScriptClass< IInput >();
	classIInput->addMethod("endOfFile", &IInput::endOfFile);
	classIInput->addMethod("readChar", &IInput::readChar);
	classIInput->addMethod("readLn", &IInput::readLn);
	scriptManager->registerClass(classIInput);

	// StreamInput
	Ref< script::AutoScriptClass< StreamInput > > classStreamInput = new script::AutoScriptClass< StreamInput >();
	classStreamInput->addConstructor< traktor::IStream*, IEncoding* >();
	scriptManager->registerClass(classStreamInput);

	// Environment
	Ref< script::AutoScriptClass< Environment > > classEnvironment = new script::AutoScriptClass< Environment >();
	classEnvironment->addConstructor();
	classEnvironment->addMethod("set", &Environment::set);
	classEnvironment->addMethod("get", &Environment::get);
	scriptManager->registerClass(classEnvironment);

	// Run
	Ref< script::AutoScriptClass< Run > > classRun = new script::AutoScriptClass< Run >();
	classRun->addConstructor();
	classRun->addMethod("cd", &Run::cd);
	classRun->addMethod("pushd", &Run::pushd);
	classRun->addMethod("popd", &Run::popd);
	classRun->addMethod("cwd", &Run::cwd);
	classRun->addMethod("run", &Run_run_1);
	classRun->addMethod("run", &Run_run_2);
	classRun->addMethod("execute", &Run_execute_1);
	classRun->addMethod("execute", &Run_execute_2);
	classRun->addMethod("stdOut", &Run::stdOut);
	classRun->addMethod("stdErr", &Run::stdErr);
	classRun->addMethod("exitCode", &Run::exitCode);
	classRun->addMethod("copy", &Run::copy);
	classRun->addMethod("replace", &Run::replace);
	classRun->addMethod("mkdir", &Run::mkdir);
	classRun->addMethod("rmdir", &Run::rmdir);
	classRun->addMethod("sleep", &Run::sleep);
	scriptManager->registerClass(classRun);

	// DateTime
	Ref< script::AutoScriptClass< DateTime > > classDateTime = new script::AutoScriptClass< DateTime >();
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
	scriptManager->registerClass(classDateTime);

	// Path
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

	// File
	Ref< script::AutoScriptClass< File > > classFile = new script::AutoScriptClass< File >();
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
	scriptManager->registerClass(classFile);

	// IVolume
	Ref< script::AutoScriptClass< IVolume > > classIVolume = new script::AutoScriptClass< IVolume >();
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
	scriptManager->registerClass(classIVolume);

	// FileSystem
	Ref< script::AutoScriptClass< FileSystem > > classFileSystem = new script::AutoScriptClass< FileSystem >();
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
	scriptManager->registerClass(classFileSystem);

	// IStream
	Ref< script::AutoScriptClass< traktor::IStream > > classIStream = new script::AutoScriptClass< traktor::IStream >();
	classIStream->addMethod("close", &traktor::IStream::close);
	classIStream->addMethod("canRead", &traktor::IStream::canRead);
	classIStream->addMethod("canWrite", &traktor::IStream::canWrite);
	classIStream->addMethod("canSeek", &traktor::IStream::canSeek);
	classIStream->addMethod("tell", &traktor::IStream::tell);
	classIStream->addMethod("available", &traktor::IStream::available);
	//classIStream->addMethod("seek", &IStream::seek);
	//classIStream->addMethod("read", &IStream::read);
	//classIStream->addMethod("write", &IStream::write);
	classIStream->addMethod("flush", &traktor::IStream::flush);
	scriptManager->registerClass(classIStream);

	// IEncoding
	Ref< script::AutoScriptClass< IEncoding > > classIEncoding = new script::AutoScriptClass< IEncoding >();
	scriptManager->registerClass(classIEncoding);

	// AnsiEncoding
	Ref< script::AutoScriptClass< AnsiEncoding > > classAnsiEncoding = new script::AutoScriptClass< AnsiEncoding >();
	classAnsiEncoding->addConstructor();
	scriptManager->registerClass(classAnsiEncoding);

	// Utf8Encoding
	Ref< script::AutoScriptClass< Utf8Encoding > > classUtf8Encoding = new script::AutoScriptClass< Utf8Encoding >();
	classUtf8Encoding->addConstructor();
	scriptManager->registerClass(classUtf8Encoding);

	// Utf16Encoding
	Ref< script::AutoScriptClass< Utf16Encoding > > classUtf16Encoding = new script::AutoScriptClass< Utf16Encoding >();
	classUtf16Encoding->addConstructor();
	scriptManager->registerClass(classUtf16Encoding);

	// Utf32Encoding
	Ref< script::AutoScriptClass< Utf32Encoding > > classUtf32Encoding = new script::AutoScriptClass< Utf32Encoding >();
	classUtf32Encoding->addConstructor();
	scriptManager->registerClass(classUtf32Encoding);

	// StringReader
	Ref< script::AutoScriptClass< StringReader > > classStringReader = new script::AutoScriptClass< StringReader >();
	classStringReader->addConstructor< traktor::IStream*, IEncoding* >();
	classStringReader->addMethod("readChar", &StringReader_readChar);
	classStringReader->addMethod("readLine", &StringReader_readLine);
	scriptManager->registerClass(classStringReader);

	// StreamCopy
	Ref< script::AutoScriptClass< StreamCopy > > classStreamCopy = new script::AutoScriptClass< StreamCopy >();
	classStreamCopy->addConstructor< traktor::IStream*, traktor::IStream* >();
	classStreamCopy->addMethod("execute", &StreamCopy::execute);
	scriptManager->registerClass(classStreamCopy);

	// Reader

	// Writer

	// IProcess
	Ref< script::AutoScriptClass< IProcess > > classIProcess = new script::AutoScriptClass< IProcess >();
	classIProcess->addMethod("getStdOut", &IProcess_getStdOut);
	classIProcess->addMethod("getStdErr", &IProcess_getStdErr);
	classIProcess->addMethod("signalCtrlC", &IProcess_signalCtrlC);
	classIProcess->addMethod("signalCtrlBreak", &IProcess_signalCtrlBreak);
	classIProcess->addMethod("exitCode", &IProcess::exitCode);
	scriptManager->registerClass(classIProcess);

	// ISharedMemory
	Ref< script::AutoScriptClass< ISharedMemory > > classISharedMemory = new script::AutoScriptClass< ISharedMemory >();
	//classISharedMemory->addMethod("read", &ISharedMemory::read);
	//classISharedMemory->addMethod("write", &ISharedMemory::write);
	classISharedMemory->addMethod("clear", &ISharedMemory::clear);
	scriptManager->registerClass(classISharedMemory);

	// OS
	Ref< script::AutoScriptClass< OS > > classOS = new script::AutoScriptClass< OS >();
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
	scriptManager->registerClass(classOS);

	// DeepClone
	Ref< script::AutoScriptClass< DeepClone > > classDeepClone = new script::AutoScriptClass< DeepClone >();
	classDeepClone->addConstructor< const ISerializable* >();
	classDeepClone->addMethod< Ref< ISerializable > >("create", &DeepClone::create);
	scriptManager->registerClass(classDeepClone);

	// DeepHash
	Ref< script::AutoScriptClass< DeepHash > > classDeepHash = new script::AutoScriptClass< DeepHash >();
	classDeepHash->addConstructor< const ISerializable* >();
	classDeepHash->addMethod("get", &DeepHash::get);
	scriptManager->registerClass(classDeepHash);

	// Property???

	// IImageFilter
	Ref< script::AutoScriptClass< drawing::IImageFilter > > classIImageFilter = new script::AutoScriptClass< drawing::IImageFilter >();
	scriptManager->registerClass(classIImageFilter);

	Ref< script::AutoScriptClass< drawing::BrightnessContrastFilter > > classBrightnessContrastFilter = new script::AutoScriptClass< drawing::BrightnessContrastFilter >();
	classBrightnessContrastFilter->addConstructor< float, float >();
	scriptManager->registerClass(classBrightnessContrastFilter);

	Ref< script::AutoScriptClass< drawing::ChainFilter > > classChainFilter = new script::AutoScriptClass< drawing::ChainFilter >();
	classChainFilter->addConstructor();
	classChainFilter->addMethod("add", &drawing::ChainFilter::add);
	scriptManager->registerClass(classChainFilter);

	Ref< script::AutoScriptClass< drawing::ConvolutionFilter > > classConvolutionFilter = new script::AutoScriptClass< drawing::ConvolutionFilter >();
	classConvolutionFilter->addStaticMethod("createGaussianBlur3", &drawing::ConvolutionFilter::createGaussianBlur3);
	classConvolutionFilter->addStaticMethod("createGaussianBlur5", &drawing::ConvolutionFilter::createGaussianBlur5);
	classConvolutionFilter->addStaticMethod("createEmboss", &drawing::ConvolutionFilter::createEmboss);
	scriptManager->registerClass(classConvolutionFilter);

	Ref< script::AutoScriptClass< drawing::CropFilter > > classCropFilter = new script::AutoScriptClass< drawing::CropFilter >();
	classCropFilter->addConstructor< int32_t, int32_t, int32_t, int32_t >(&drawing_CropFilter_constructor);
	scriptManager->registerClass(classCropFilter);

	Ref< script::AutoScriptClass< drawing::DilateFilter > > classDilateFilter = new script::AutoScriptClass< drawing::DilateFilter >();
	classDilateFilter->addConstructor< int32_t >();
	scriptManager->registerClass(classDilateFilter);

	Ref< script::AutoScriptClass< drawing::GammaFilter > > classGammaFilter = new script::AutoScriptClass< drawing::GammaFilter >();
	classGammaFilter->addConstructor< float >();
	//classGammaFilter->addConstructor< float, float, float, float >();
	scriptManager->registerClass(classGammaFilter);

	Ref< script::AutoScriptClass< drawing::GrayscaleFilter > > classGrayscaleFilter = new script::AutoScriptClass< drawing::GrayscaleFilter >();
	classGrayscaleFilter->addConstructor();
	scriptManager->registerClass(classGrayscaleFilter);

	Ref< script::AutoScriptClass< drawing::MirrorFilter > > classMirrorFilter = new script::AutoScriptClass< drawing::MirrorFilter >();
	classMirrorFilter->addConstructor< bool, bool >();
	scriptManager->registerClass(classMirrorFilter);

	Ref< script::AutoScriptClass< drawing::NormalizeFilter > > classNormalizeFilter = new script::AutoScriptClass< drawing::NormalizeFilter >();
	classNormalizeFilter->addConstructor();
	scriptManager->registerClass(classNormalizeFilter);

	Ref< script::AutoScriptClass< drawing::NormalMapFilter > > classNormalMapFilter = new script::AutoScriptClass< drawing::NormalMapFilter >();
	classNormalMapFilter->addConstructor< float >();
	scriptManager->registerClass(classNormalMapFilter);

	Ref< script::AutoScriptClass< drawing::PerlinNoiseFilter > > classPerlinNoiseFilter = new script::AutoScriptClass< drawing::PerlinNoiseFilter >();
	classPerlinNoiseFilter->addConstructor< int, float, float >();
	scriptManager->registerClass(classPerlinNoiseFilter);

	Ref< script::AutoScriptClass< drawing::PremultiplyAlphaFilter > > classPremultiplyAlphaFilter = new script::AutoScriptClass< drawing::PremultiplyAlphaFilter >();
	classPremultiplyAlphaFilter->addConstructor();
	scriptManager->registerClass(classPremultiplyAlphaFilter);

	Ref< script::AutoScriptClass< drawing::QuantizeFilter > > classQuantizeFilter = new script::AutoScriptClass< drawing::QuantizeFilter >();
	classQuantizeFilter->addConstructor< int >();
	scriptManager->registerClass(classQuantizeFilter);

	Ref< script::AutoScriptClass< drawing::ScaleFilter > > classScaleFilter = new script::AutoScriptClass< drawing::ScaleFilter >();
	classScaleFilter->addConstructor< int32_t, int32_t >(&drawing_ScaleFilter_constructor);
	scriptManager->registerClass(classScaleFilter);

	Ref< script::AutoScriptClass< drawing::SharpenFilter > > classSharpenFilter = new script::AutoScriptClass< drawing::SharpenFilter >();
	classSharpenFilter->addConstructor< int, float >();
	scriptManager->registerClass(classSharpenFilter);

	Ref< script::AutoScriptClass< drawing::SwizzleFilter > > classSwizzleFilter = new script::AutoScriptClass< drawing::SwizzleFilter >();
	classSwizzleFilter->addConstructor< const std::wstring& >();
	scriptManager->registerClass(classSwizzleFilter);

	Ref< script::AutoScriptClass< drawing::TonemapFilter > > classTonemapFilter = new script::AutoScriptClass< drawing::TonemapFilter >();
	classTonemapFilter->addConstructor();
	scriptManager->registerClass(classTonemapFilter);

	Ref< script::AutoScriptClass< drawing::TransformFilter > > classTransformFilter = new script::AutoScriptClass< drawing::TransformFilter >();
	classTransformFilter->addConstructor< const Color4f&, const Color4f& >();
	scriptManager->registerClass(classTransformFilter);

	// Image
	Ref< script::AutoScriptClass< drawing::Image > > classImage = new script::AutoScriptClass< drawing::Image >();
	classImage->addConstructor();
	classImage->addMethod< Ref< drawing::Image > >("clone", &drawing::Image::clone);
	classImage->addMethod< Ref< drawing::Image >, bool >("clone", &drawing::Image::clone);
	classImage->addMethod("copy", &drawing::Image::copy);
	classImage->addMethod("clear", &drawing::Image::clear);
	classImage->addMethod("getPixel", &drawing_Image_getPixel);
	classImage->addMethod("setPixel", &drawing::Image::setPixel);
	classImage->addMethod("apply", &drawing::Image::apply);
	classImage->addStaticMethod("loadFromFile", &drawing_Image_loadFromFile);
	classImage->addStaticMethod("loadFromStream", &drawing_Image_loadFromStream);
	classImage->addMethod("save", &drawing_Image_save);
	classImage->addMethod("getWidth", &drawing::Image::getWidth);
	classImage->addMethod("getHeight", &drawing::Image::getHeight);
	scriptManager->registerClass(classImage);

	// Xml
	Ref< script::AutoScriptClass< xml::Attribute > > classAttribute = new script::AutoScriptClass< xml::Attribute >();
	classAttribute->addConstructor< const std::wstring&, const std::wstring& >();
	classAttribute->addMethod("getName", &xml::Attribute::getName);
	classAttribute->addMethod("setName", &xml::Attribute::setName);
	classAttribute->addMethod("getValue", &xml::Attribute::getValue);
	classAttribute->addMethod("setValue", &xml::Attribute::setValue);
	classAttribute->addMethod("getPrevious", &xml::Attribute::getPrevious);
	classAttribute->addMethod("getNext", &xml::Attribute::getNext);
	scriptManager->registerClass(classAttribute);

	Ref< script::AutoScriptClass< xml::Node > > classNode = new script::AutoScriptClass< xml::Node >();
	classNode->addMethod("getName", &xml::Node::getName);
	classNode->addMethod("setName", &xml::Node::setName);
	classNode->addMethod("getValue", &xml::Node::getValue);
	classNode->addMethod("setValue", &xml::Node::setValue);
	classNode->addMethod("write", &xml_Node_write);
	classNode->addMethod("unlink", &xml::Node::unlink);
	classNode->addMethod("addChild", &xml::Node::addChild);
	classNode->addMethod("removeChild", &xml::Node::removeChild);
	classNode->addMethod("removeAllChildren", &xml::Node::removeAllChildren);
	classNode->addMethod("insertBefore", &xml::Node::insertBefore);
	classNode->addMethod("insertAfter", &xml::Node::insertAfter);
	classNode->addMethod("getParent", &xml::Node::getParent);
	classNode->addMethod("getPreviousSibling", &xml::Node::getPreviousSibling);
	classNode->addMethod("getNextSibling", &xml::Node::getNextSibling);
	classNode->addMethod("getFirstChild", &xml::Node::getFirstChild);
	classNode->addMethod("getLastChild", &xml::Node::getLastChild);
	scriptManager->registerClass(classNode);

	Ref< script::AutoScriptClass< xml::Text > > classText = new script::AutoScriptClass< xml::Text >();
	classText->addConstructor< const std::wstring& >();
	classText->addMethod("getValue", &xml::Text::getValue);
	classText->addMethod("setValue", &xml::Text::setValue);
	classText->addMethod("clone", &xml::Text::clone);
	scriptManager->registerClass(classText);

	Ref< script::AutoScriptClass< xml::Element > > classElement = new script::AutoScriptClass< xml::Element >();
	classElement->addConstructor< const std::wstring& >();
	classElement->addMethod("get", &xml_Element_get);
	classElement->addMethod("getSingle", &xml::Element::getSingle);
	classElement->addMethod("getPath", &xml::Element::getPath);
	classElement->addMethod("match", &xml::Element::match);
	classElement->addMethod("hasAttribute", &xml::Element::hasAttribute);
	classElement->addMethod("setAttribute", &xml::Element::setAttribute);
	classElement->addMethod("getFirstAttribute", &xml::Element::getFirstAttribute);
	classElement->addMethod("getLastAttribute", &xml::Element::getLastAttribute);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_1);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_2);
	classElement->addMethod("getChildElementByName", &xml::Element::getChildElementByName);
	classElement->addMethod("clone", &xml::Element::clone);
	scriptManager->registerClass(classElement);

	Ref< script::AutoScriptClass< xml::Document > > classDocument = new script::AutoScriptClass< xml::Document >();
	classDocument->addConstructor();
	classDocument->addMethod("loadFromFile", &xml::Document::loadFromFile);
	classDocument->addMethod("loadFromStream", &xml::Document::loadFromStream);
	classDocument->addMethod("loadFromText", &xml::Document::loadFromText);
	classDocument->addMethod("saveAsFile", &xml::Document::saveAsFile);
	classDocument->addMethod("saveIntoStream", &xml::Document::saveIntoStream);
	classDocument->addMethod("get", &xml_Document_get);
	classDocument->addMethod("setDocumentElement", &xml::Document::setDocumentElement);
	classDocument->addMethod("getDocumentElement", &xml::Document::getDocumentElement);
	classDocument->addMethod("clone", &xml::Document::clone);
	scriptManager->registerClass(classDocument);

	// HtmlDocument

	// Socket
	Ref< script::AutoScriptClass< net::Socket > > classSocket = new script::AutoScriptClass< net::Socket >();
	classSocket->addMethod("close", &net::Socket::close);
	classSocket->addMethod("select", &net::Socket::select);
	//classSocket->addMethod("send", &net::Socket::send);
	//classSocket->addMethod("recv", &net::Socket::recv);
	//classSocket->addMethod("send", &net::Socket::send);
	//classSocket->addMethod("recv", &net::Socket::recv);
	scriptManager->registerClass(classSocket);

	// TcpSocket
	Ref< script::AutoScriptClass< net::TcpSocket > > classTcpSocket = new script::AutoScriptClass< net::TcpSocket >();
	classTcpSocket->addConstructor();
	//classTcpSocket->addMethod("bind", &net::TcpSocket::bind);
	//classTcpSocket->addMethod("bind", &net::TcpSocket::bind);
	//classTcpSocket->addMethod("connect", &net::TcpSocket::connect);
	//classTcpSocket->addMethod("connect", &net::TcpSocket::connect);
	classTcpSocket->addMethod("listen", &net::TcpSocket::listen);
	classTcpSocket->addMethod("accept", &net::TcpSocket::accept);
	classTcpSocket->addMethod("getLocalAddress", &net::TcpSocket::getLocalAddress);
	classTcpSocket->addMethod("getRemoteAddress", &net::TcpSocket::getRemoteAddress);
	classTcpSocket->addMethod("setNoDelay", &net::TcpSocket::setNoDelay);
	scriptManager->registerClass(classTcpSocket);

	// UdpSocket
	Ref< script::AutoScriptClass< net::UdpSocket > > classUdpSocket = new script::AutoScriptClass< net::UdpSocket >();
	classUdpSocket->addConstructor();
	//classUdpSocket->addMethod("bind", &net::UdpSocket::bind);
	//classUdpSocket->addMethod("bind", &net::UdpSocket::bind);
	//classUdpSocket->addMethod("connect", &net::UdpSocket::connect);
	//classUdpSocket->addMethod("connect", &net::UdpSocket::connect);
	//classUdpSocket->addMethod("sendTo", &net::UdpSocket::send);
	//classUdpSocket->addMethod("recvFrom", &net::UdpSocket::recv);
	scriptManager->registerClass(classUdpSocket);

	// MulticastUdpSocket
	Ref< script::AutoScriptClass< net::MulticastUdpSocket > > classMulticastUdpSocket = new script::AutoScriptClass< net::MulticastUdpSocket >();
	scriptManager->registerClass(classMulticastUdpSocket);

	// SocketAddress
	Ref< script::AutoScriptClass< net::SocketAddress > > classSocketAddress = new script::AutoScriptClass< net::SocketAddress >();
	scriptManager->registerClass(classSocketAddress);

	// SocketAddressIPv4
	Ref< script::AutoScriptClass< net::SocketAddressIPv4 > > classSocketAddressIPv4 = new script::AutoScriptClass< net::SocketAddressIPv4 >();
	scriptManager->registerClass(classSocketAddressIPv4);

	// SocketAddressIPv6
	Ref< script::AutoScriptClass< net::SocketAddressIPv6 > > classSocketAddressIPv6 = new script::AutoScriptClass< net::SocketAddressIPv6 >();
	scriptManager->registerClass(classSocketAddressIPv6);

	// SocketSet
	Ref< script::AutoScriptClass< net::SocketSet > > classSocketSet = new script::AutoScriptClass< net::SocketSet >();
	scriptManager->registerClass(classSocketSet);

	// Url
	Ref< script::AutoScriptClass< net::Url > > classUrl = new script::AutoScriptClass< net::Url >();
	classUrl->addConstructor();
	classUrl->addConstructor< const std::wstring& >();
	classUrl->addMethod("set", &net::Url::set);
	classUrl->addMethod("valid", &net::Url::valid);
	classUrl->addMethod("getDefaultPort", &net::Url::getDefaultPort);
	classUrl->addMethod("getFile", &net::Url::getFile);
	classUrl->addMethod("getHost", &net::Url::getHost);
	classUrl->addMethod("getPath", &net::Url::getPath);
	classUrl->addMethod("getPort", &net::Url::getPort);
	classUrl->addMethod("getProtocol", &net::Url::getProtocol);
	classUrl->addMethod("getQuery", &net::Url::getQuery);
	classUrl->addMethod("getRef", &net::Url::getRef);
	classUrl->addMethod("getUserInfo", &net::Url::getUserInfo);
	classUrl->addMethod("getString", &net::Url::getString);
	//classUrl->addStaticMethod("encode", &net::Url::encode);
	//classUrl->addStaticMethod("encode", &net::Url::encode);
	//classUrl->addStaticMethod("decode", &net::Url::decode);
	scriptManager->registerClass(classUrl);

	// UrlConnection
	Ref< script::AutoScriptClass< net::UrlConnection > > classUrlConnection = new script::AutoScriptClass< net::UrlConnection >();
	classUrlConnection->addStaticMethod("open", &net::UrlConnection::open);
	classUrlConnection->addMethod("getUrl", &net::UrlConnection::getUrl);
	classUrlConnection->addMethod("getStream", &net::UrlConnection::getStream);
	scriptManager->registerClass(classUrlConnection);

	// HttpRequest
	Ref< script::AutoScriptClass< net::HttpRequest > > classHttpRequest = new script::AutoScriptClass< net::HttpRequest >();
	classHttpRequest->addMethod("getMethod", &net_HttpRequest_getMethod);
	classHttpRequest->addMethod("getResource", &net::HttpRequest::getResource);
	classHttpRequest->addMethod("hasValue", &net::HttpRequest::hasValue);
	classHttpRequest->addMethod("setValue", &net::HttpRequest::setValue);
	classHttpRequest->addMethod("getValue", &net::HttpRequest::getValue);
	classHttpRequest->addStaticMethod("parse", &net::HttpRequest::parse);
	scriptManager->registerClass(classHttpRequest);

	// HttpServer::IRequestListener
	Ref< script::AutoScriptClass< net::HttpServer::IRequestListener > > classHttpServer_IRequestListener = new script::AutoScriptClass< net::HttpServer::IRequestListener >();
	scriptManager->registerClass(classHttpServer_IRequestListener);

	// HttpServer
	Ref< script::AutoScriptClass< net::HttpServer > > classHttpServer = new script::AutoScriptClass< net::HttpServer >();
	classHttpServer->addConstructor();
	classHttpServer->addMethod("create", &net_HttpServer_create);
	classHttpServer->addMethod("destroy", &net::HttpServer::destroy);
	classHttpServer->addMethod("setRequestListener", &net_HttpServer_setRequestListener);
	classHttpServer->addMethod("update", &net::HttpServer::update);
	scriptManager->registerClass(classHttpServer);

	// IResultSet
	Ref< script::AutoScriptClass< sql::IResultSet > > classIResultSet = new script::AutoScriptClass< sql::IResultSet >();
	classIResultSet->addMethod("next", &sql::IResultSet::next);
	classIResultSet->addMethod("getColumnCount", &sql::IResultSet::getColumnCount);
	classIResultSet->addMethod("getColumnName", &sql::IResultSet::getColumnName);
	classIResultSet->addMethod< int32_t, int32_t >("getInt32", &sql::IResultSet::getInt32);
	//classIResultSet->addMethod< int64_t, int32_t >("getInt64", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, int32_t >("getFloat", &sql::IResultSet::getFloat);
	//classIResultSet->addMethod< double, int32_t >("getDouble", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, int32_t >("getString", &sql::IResultSet::getString);
	classIResultSet->addMethod("findColumn", &sql::IResultSet::findColumn);
	classIResultSet->addMethod< int32_t, const std::wstring& >("getInt32ByName", &sql::IResultSet::getInt32);
	//classIResultSet->addMethod< int64_t, const std::wstring& >("getInt64ByName", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, const std::wstring& >("getFloatByName", &sql::IResultSet::getFloat);
	//classIResultSet->addMethod< double, const std::wstring& >("getDoubleByName", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, const std::wstring& >("getStringByName", &sql::IResultSet::getString);
	scriptManager->registerClass(classIResultSet);

	// IConnection
	Ref< script::AutoScriptClass< sql::IConnection > > classIConnection = new script::AutoScriptClass< sql::IConnection >();
	classIConnection->addMethod("connect", &sql::IConnection::connect);
	classIConnection->addMethod("disconnect", &sql::IConnection::disconnect);
	classIConnection->addMethod("executeQuery", &sql::IConnection::executeQuery);
	classIConnection->addMethod("executeUpdate", &sql::IConnection::executeUpdate);
	classIConnection->addMethod("lastInsertId", &sql::IConnection::lastInsertId);
	classIConnection->addMethod("tableExists", &sql::IConnection::tableExists);
	scriptManager->registerClass(classIConnection);

	// ConnectionSqlite3
	Ref< script::AutoScriptClass< sql::ConnectionSqlite3 > > classConnectionSqlite3 = new script::AutoScriptClass< sql::ConnectionSqlite3 >();
	classConnectionSqlite3->addConstructor();
	scriptManager->registerClass(classConnectionSqlite3);

	return scriptManager;
}

int32_t executeRun(script::IScriptManager* scriptManager, const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	// Compile script into a runnable resource.
	Ref< script::IScriptResource > scriptResource = scriptManager->compile(fileName.getPathName(), text, 0, 0);
	if (!scriptResource)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create an empty context which we use as a template for actual script context;
	// because we want globals to be accessible at the creation of the script context.
	Ref< script::IScriptContext > globalContext = scriptManager->createContext(0, 0);
	if (!globalContext)
	{
		log::error << L"Unable to create global script context" << Endl;
		return 1;
	}

	Ref< Environment > environment = new Environment(OS::getInstance().getEnvironment());
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	globalContext->setGlobal("environment", script::Any::fromObject(environment));
	globalContext->setGlobal("run", script::Any::fromObject(new Run()));
	globalContext->setGlobal("fileSystem", script::Any::fromObject(&FileSystem::getInstance()));
	globalContext->setGlobal("os", script::Any::fromObject(&OS::getInstance()));
	globalContext->setGlobal("stdout", script::Any::fromObject(new StdOutput(stdout)));
	globalContext->setGlobal("stderr", script::Any::fromObject(new StdOutput(stderr)));

	// Create script context; some scripts might actually do it's work while
	// being constructed which is perfectly valid.
	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(scriptResource, globalContext);
	if (!scriptContext)
		return 1;

	// Execute optional "main" function.
	script::Any retval = script::Any::fromInteger(0);
	if (scriptContext->haveFunction("main"))
	{
		// Transform arguments into script array.
		std::vector< std::wstring > args;
		for (int32_t i = 1; i < cmdLine.getCount(); ++i)
			args.push_back(cmdLine.getString(i));

		script::Any a = script::CastAny< std::vector< std::wstring > >::set(args);
		retval = scriptContext->executeFunction("main", 1, &a);
	}

	safeDestroy(scriptContext);
	safeDestroy(globalContext);

	return retval.getInteger();
}

int32_t executeTemplate(script::IScriptManager* scriptManager, const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	Ref< ProduceOutput > o = new ProduceOutput();

	// Transform into valid script.
	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		size_t s = text.find(L"<!--", offset);
		if (s == text.npos)
			break;

		size_t e = text.find(L"--!>", s);
		if (e == text.npos)
		{
			log::error << L"Template syntax error; missing end" << Endl;
			return 1;
		}

		int32_t id = o->addSection(text.substr(offset, s - offset));
		ss << L"output:printSection(" << id << L")" << Endl;
		ss << text.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(text.substr(offset));
	ss << L"output:printSection(" << id << L")" << Endl;

	Ref< script::IScriptResource > scriptResource = scriptManager->compile(fileName.getPathName(), ss.str(), 0, 0);
	if (!scriptResource)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create an empty context which we use as a template for actual script context;
	// because we want globals to be accessible at the creation of the script context.
	Ref< script::IScriptContext > globalContext = scriptManager->createContext(0, 0);
	if (!globalContext)
	{
		log::error << L"Unable to create global script context" << Endl;
		return 1;
	}

	// Transform arguments into script array.
	std::vector< std::wstring > args;
	for (int32_t i = 1; i < cmdLine.getCount(); ++i)
		args.push_back(cmdLine.getString(i));

	Ref< Environment > environment = new Environment(OS::getInstance().getEnvironment());
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	globalContext->setGlobal("environment", script::Any::fromObject(environment));
	globalContext->setGlobal("run", script::Any::fromObject(new Run()));
	globalContext->setGlobal("fileSystem", script::Any::fromObject(&FileSystem::getInstance()));
	globalContext->setGlobal("os", script::Any::fromObject(&OS::getInstance()));
	globalContext->setGlobal("stdout", script::Any::fromObject(new StdOutput(stdout)));
	globalContext->setGlobal("stderr", script::Any::fromObject(new StdOutput(stderr)));
	globalContext->setGlobal("output", script::Any::fromObject(o));
	globalContext->setGlobal("args", script::CastAny< std::vector< std::wstring > >::set(args));

	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(scriptResource, globalContext);
	if (!scriptContext)
		return 1;

	safeDestroy(scriptContext);
	safeDestroy(globalContext);

	log::info << o->getProduct() << Endl;

	return 0;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Run 1.3.3" << Endl;
		log::info << Endl;
		log::info << L"Usage: Run (option(s)) [<file>.run|<file>.template] (args ...)" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -as-run                   Run file as run" << Endl;
		log::info << L"    -as-template              Run file as template" << Endl;
		log::info << Endl;
		log::info << L"  For .run files:" << Endl;
		log::info << L"    -e,-entry-point=function  Script entry point (default \"main\")" << Endl;
		log::info << Endl;
		log::info << L"  For .template files:" << Endl;
		log::info << L"    -o,-output=filename       Output file (default stdout)" << Endl;
		return 0;
	}

	Path fileName = cmdLine.getString(0);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
	{
		log::error << L"Failed to open \"" << fileName.getPathName() << L"\"" << Endl;
		return 1;
	}

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	safeClose(file);

	std::wstring text = ss.str();

	Ref< script::IScriptManager > scriptManager = createScriptManager();
	if (!scriptManager)
	{
		log::error << L"Failed to create script manager" << Endl;
		return 1;
	}

	net::Network::initialize();

	int32_t result = 1;

	bool explicitRun = cmdLine.hasOption(L"as-run");
	bool explicitTemplate = cmdLine.hasOption(L"as-template");

	if ((explicitRun && !explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"run") == 0)
		result = executeRun(scriptManager, text, fileName, cmdLine);
	else if ((!explicitRun && explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"template") == 0)
		result = executeTemplate(scriptManager, text, fileName, cmdLine);
	else
		log::error << L"Unknown file type \"" << fileName.getExtension() << L"\"; must be either \"run\" or \"template\"" << Endl;

	net::Network::finalize();

	safeDestroy(scriptManager);
	return result;
}
