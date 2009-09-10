#include <windows.h>
#include <rapi.h>

#include <Core/Io/FileSystem.h>
#include <Core/Io/Stream.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

using namespace traktor;

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

	hr = CeRapiInit();
	if (FAILED(hr) && hr != CERAPI_E_ALREADYINITIALIZED)
	{
		log::error << L"Unable to initialize RAPI" << Endl;
		return 1;
	}

	std::wstring targetPath = cmdLine.getString(0);
	uint32_t failureCount = 0;

	CeCreateDirectory(
		targetPath.c_str(),
		NULL
	);

	for (int i = 1; i < cmdLine.getCount(); ++i)
	{
		Path sourceFile = cmdLine.getString(i);

		log::info << L"Deploying file \"" << sourceFile.getFileName() << L"\"..." << Endl;

		Ref< Stream > file = FileSystem::getInstance().open(sourceFile, File::FmRead);
		if (!file)
		{
			log::error << L"Unable to open source file \"" << sourceFile.getPathName() << L"\"" << Endl;
			failureCount++;
			continue;
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
			failureCount++;
			continue;
		}

		uint8_t buffer[1024];
		while (file->available())
		{
			int nb = file->read(buffer, sizeof(buffer));
			if (nb < 0)
			{
				log::error << L"Read error; target file might be incomplete" << Endl;
				failureCount++;
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
				failureCount++;
				break;
			}

			if (nbw != DWORD(nb))
			{
				log::error << L"Write error (2); target file might be incomplete" << Endl;
				failureCount++;
				break;
			}
		}

		CeCloseHandle(hTargetFile);

		file->close();
	}

	CeRapiUninit();

	return failureCount ? 2 : 0;
}
