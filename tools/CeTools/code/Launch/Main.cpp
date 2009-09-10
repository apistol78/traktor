#include <windows.h>
#include <rapi.h>

#include <Core/Io/StringOutputStream.h>
#include <Core/Misc/TString.h>
#include <Core/Log/Log.h>

using namespace traktor;

// CeLaunch target-path commands...
int main(int argc, const char** argv)
{
	HRESULT hr;

	if (argc <= 1)
	{
		log::info << L"Usage: CeLaunch target-path (command line)" << Endl;
		return 0;
	}

	hr = CeRapiInit();
	if (FAILED(hr) && hr != CERAPI_E_ALREADYINITIALIZED)
	{
		log::error << L"Unable to initialize RAPI" << Endl;
		return 1;
	}

	std::wstring targetFile = mbstows(argv[1]);

	StringOutputStream ss;
	for (int i = 2; i < argc; ++i)
		ss << ((i > 2) ? L" " : L"") << mbstows(argv[i]);

	std::wstring targetCommandLine = ss.str();

	PROCESS_INFORMATION pi;
	std::memset(&pi, 0, sizeof(pi));

	BOOL result = CeCreateProcess(
		targetFile.c_str(),
		targetCommandLine.empty() ? NULL : targetCommandLine.c_str(),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		NULL,
		&pi
	);
	if (!result)
	{
		log::error << L"Unable to create remote process \"" << targetCommandLine << L"\"" << Endl;
		return 1;
	}

	log::info << L"Remote process launched successfully" << Endl;

	CeCloseHandle(pi.hProcess);
	CeCloseHandle(pi.hThread);

	CeRapiUninit();
	return 0;
}
