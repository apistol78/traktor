#include <windows.h>
#include <rapi.h>

#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

using namespace traktor;

void deployFile(const Path& sourceFile, const std::wstring& targetPath)
{
	log::info << L"Deploying file \"" << sourceFile.getFileName() << L"\"..." << Endl;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(sourceFile, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to open source file \"" << sourceFile.getPathName() << L"\"" << Endl;
		return;
	}

	std::wstring targetFile = targetPath + L"\\" + sourceFile.getFileName();

	HANDLE hTargetFile = CeCreateFile(
		targetFile.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hTargetFile == INVALID_HANDLE_VALUE)
	{
		log::error << L"Unable to create target file \"" << targetFile << L"\"" << Endl;
		return;
	}

	uint8_t buffer[1024];
	while (file->available())
	{
		int nb = file->read(buffer, sizeof(buffer));
		if (nb < 0)
		{
			log::error << L"Read error; target file might be incomplete" << Endl;
			break;
		}

		DWORD nbw = 0;
		if (!CeWriteFile(
			hTargetFile,
			buffer,
			nb,
			&nbw,
			NULL
		))
		{
			log::error << L"Write error (1); target file might be incomplete" << Endl;
			break;
		}

		if (nbw != DWORD(nb))
		{
			log::error << L"Write error (2); target file might be incomplete" << Endl;
			break;
		}
	}

	CeCloseHandle(hTargetFile);

	file->close();
}

void deployFiles(const Path& sourcePath, const std::wstring& targetPath, bool recursive)
{
	RefArray< File > files;
	FileSystem::getInstance().find(sourcePath, files);
	for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
	{
		Path sourceFile = (*i)->getPath();
		if ((*i)->isDirectory())
		{
			if (recursive)
			{
				if (sourceFile.getPathName() != L"." && sourceFile.getPathName() != L"..")
					deployFiles(sourceFile, targetPath, true);
			}
		}
		else
		{
			deployFile(sourceFile, targetPath);
		}
	}
}

// CeDeploy target-path file (file) ...
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	HRESULT hr;

	if (cmdLine.getCount() <= 1)
	{
		log::info << L"Usage: CeDeploy target-path file (file)" << Endl;
		return 0;
	}

	bool recursive = cmdLine.hasOption('r');

	hr = CeRapiInit();
	if (FAILED(hr) && hr != CERAPI_E_ALREADYINITIALIZED)
	{
		log::error << L"Unable to initialize RAPI" << Endl;
		return 1;
	}

	std::wstring targetPath = cmdLine.getString(0);

	CeCreateDirectory(
		targetPath.c_str(),
		NULL
	);

	for (size_t i = 1; i < cmdLine.getCount(); ++i)
	{
		Path sourcePath = cmdLine.getString(i);
		deployFiles(sourcePath, targetPath, recursive);
	}

	CeRapiUninit();

	return 0;
}
