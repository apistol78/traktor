#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Run/App/Run.h"

namespace traktor
{
	namespace run
	{

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

int32_t Run::run(const std::wstring& command, const std::wstring& saveOutputAs)
{
	Path executable = OS::getInstance().getExecutable();
	return execute(executable.getPathName() + L" " + command, saveOutputAs);
}

int32_t Run::execute(const std::wstring& command, const std::wstring& saveOutputAs)
{
	Ref< IProcess > process = OS::getInstance().execute(command, cwd(), 0, true, true, false);
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

	}
}
