/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Run/App/Run.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace run
	{
		namespace
		{

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
		return xml::XmlDeserializer(file).readObject< PropertyGroup >();
	else
		return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.Run", Run, Object)

Run::Run()
:	m_exitCode(0)
{
	m_cwd.push_back(L"");
}

void Run::cd(const std::wstring& path)
{
	m_cwd.back() = path;
}

void Run::pushd(const std::wstring& path)
{
	m_cwd.push_back(path);
}

void Run::popd()
{
	m_cwd.pop_back();
	if (m_cwd.empty())
		m_cwd.push_back(L"");
}

std::wstring Run::cwd() const
{
	return m_cwd.back();
}

int32_t Run::run(const std::wstring& command, const std::wstring& saveOutputAs, const Environment* env)
{
	Path executable = OS::getInstance().getExecutable();
	return execute(executable.getPathName() + L" " + command, saveOutputAs, env);
}

int32_t Run::execute(const std::wstring& command, const std::wstring& saveOutputAs, const Environment* env)
{
	Ref< IProcess > process = OS::getInstance().execute(command, cwd(), env, true, true, false);
	if (!process)
		return -1;

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	Ref< FileOutputStream > fileOutput;
	bool nullOutput = false;

	if (!saveOutputAs.empty())
	{
		if (saveOutputAs != L"(null)")
		{
			Ref< traktor::IStream > output = FileSystem::getInstance().open(saveOutputAs, File::FmWrite);
			if (!output)
				return -1;

			fileOutput = new FileOutputStream(output, new Utf8Encoding());
		}
		else
			nullOutput = true;
	}
	
	StringOutputStream stdOut;
	StringOutputStream stdErr;
	std::wstring str;
	for (;;)
	{
		PipeReader::Result result1 = stdOutReader.readLine(str, 10);
		if (result1 == PipeReader::RtOk)
		{
			if (fileOutput)
				(*fileOutput) << str << Endl;
			else if (!nullOutput)
				log::info << str << Endl;

			stdOut << str << Endl;
		}

		PipeReader::Result result2 = stdErrReader.readLine(str, 10);
		if (result2 == PipeReader::RtOk)
		{
			if (!nullOutput)
				log::error << str << Endl;
			stdErr << str << Endl;
		}

		if (result1 == PipeReader::RtEnd && result2 == PipeReader::RtEnd)
			break;
	}

	safeClose(fileOutput);

	m_stdOut = stdOut.str();
	m_stdErr = stdErr.str();
	m_exitCode = process->exitCode();

	return m_exitCode;
}

const std::wstring& Run::stdOut() const
{
	return m_stdOut;
}

const std::wstring& Run::stdErr() const
{
	return m_stdErr;
}

int32_t Run::exitCode() const
{
	return m_exitCode;
}

bool Run::exist(const std::wstring& path)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().exist(sourcePath);
}

bool Run::rm(const std::wstring& path)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), path);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);
	for (RefArray< File >::const_iterator i = sourceFiles.begin(); i != sourceFiles.end(); ++i)
	{
		if (!FileSystem::getInstance().remove((*i)->getPath()))
			return false;
	}

	return true;
}

bool Run::copy(const std::wstring& source, const std::wstring& target)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), source);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);
	for (RefArray< File >::const_iterator i = sourceFiles.begin(); i != sourceFiles.end(); ++i)
	{
		if (!FileSystem::getInstance().copy(target, (*i)->getPath(), false))
			return false;
	}

	return true;
}

bool Run::replace(const std::wstring& source, const std::wstring& target)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), source);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);
	for (RefArray< File >::const_iterator i = sourceFiles.begin(); i != sourceFiles.end(); ++i)
	{
		if (!FileSystem::getInstance().copy(target, (*i)->getPath(), true))
			return false;
	}

	return true;
}

bool Run::mkdir(const std::wstring& path)
{
	Path fullPath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().makeAllDirectories(fullPath);
}

bool Run::rmdir(const std::wstring& path)
{
	Path fullPath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().removeDirectory(fullPath);
}

void Run::sleep(int32_t ms)
{
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

Any Run::getProperty(const std::wstring& fileName1, const std::wstring& fileName2, const std::wstring& propertyName, const Any& defaultValue) const
{
	Ref< PropertyGroup > p = loadSettings(fileName1);
	if (!p)
		return Any();

	if (!fileName2.empty())
	{
		Ref< PropertyGroup > pr = loadSettings(fileName2);
		if (!pr)
			return Any();

		if ((p = p->merge(pr, PropertyGroup::MmReplace)) == 0)
			return Any();
	}

	Ref< IPropertyValue > property = p->getProperty(propertyName);
	if (!property)
		return Any();

	if (const PropertyBoolean* propertyBoolean = dynamic_type_cast< const PropertyBoolean* >(property))
		return Any::fromBoolean(*propertyBoolean);
	else if (const PropertyInteger* propertyInteger = dynamic_type_cast< const PropertyInteger* >(property))
		return Any::fromInt32(*propertyInteger);
	else if (const PropertyFloat* propertyFloat = dynamic_type_cast< const PropertyFloat* >(property))
		return Any::fromFloat(*propertyFloat);
	else if (const PropertyString* propertyString = dynamic_type_cast< const PropertyString* >(property))
		return Any::fromString(*propertyString);
	else
		return Any::fromObject(property);
}

bool Run::setProperty(const std::wstring& fileName, const std::wstring& propertyName, const Any& value) const
{
	Ref< PropertyGroup > p = loadSettings(fileName);
	if (!p)
		p = new PropertyGroup();

	if (value.isBoolean())
		p->setProperty< PropertyBoolean >(propertyName, value.getBoolean());
	else if (value.isInt32())
		p->setProperty< PropertyInteger >(propertyName, value.getInt32());
	else if (value.isInt64())
		p->setProperty< PropertyInteger >(propertyName, (int32_t)value.getInt64());
	else if (value.isFloat())
		p->setProperty< PropertyFloat >(propertyName, value.getFloat());
	else if (value.isString())
		p->setProperty< PropertyString >(propertyName, value.getWideString());
	else if (is_a< IPropertyValue >(value.getObject()))
		p->setProperty(propertyName, static_cast< IPropertyValue* >(value.getObjectUnsafe()));
	else
		return false;

	Ref< IStream > f = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (f)
		return xml::XmlSerializer(f).writeObject(p);
	else
		return false;
}

	}
}
