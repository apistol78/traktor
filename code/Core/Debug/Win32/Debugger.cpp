#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <csignal>
#include "Core/Debug/Debugger.h"
#include "Core/Debug/Win32/Resource.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

struct DialogParams
{
	std::string expression;
	std::string file;
	int line;
	std::wstring message;
};

INT_PTR CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			DialogParams* params = reinterpret_cast< DialogParams* >(lParam);
			std::stringstream ss;
			ss << params->file << " (" << params->line << ")";
			SetDlgItemText(hWnd, IDC_EXPRESSION, mbstots(params->expression).c_str());
			SetDlgItemText(hWnd, IDC_FILE, mbstots(ss.str()).c_str());
			SetDlgItemText(hWnd, IDC_MESSAGE, wstots(params->message).c_str());
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam)
		{
		case ID_BREAK:
		case ID_IGNORE:
		case ID_ABORT:
			EndDialog(hWnd, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

	}

Debugger& Debugger::getInstance()
{
	static Debugger instance;
	return instance;
}

void Debugger::assertionFailed(const std::string& expression, const std::string& file, int line, const std::wstring& message)
{
#if !defined(T_STATIC)
#if defined(_DEBUG)
	HINSTANCE hInstance = GetModuleHandle(_T("Traktor.Core_d.dll"));
#else
	HINSTANCE hInstance = GetModuleHandle(_T("Traktor.Core.dll"));
#endif
#else
	HINSTANCE hInstance = GetModuleHandle(NULL);
#endif

	DialogParams params = { expression, file, line, message };

	INT_PTR result = DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ASSERT_DIALOG),
		NULL,
		DialogProc,
		reinterpret_cast< LPARAM >(&params)
	);

	if (result == ID_BREAK)
		__debugbreak();
	else if (result == ID_ABORT)
	{
		raise(SIGABRT);
		exit(3);
	}
}

void Debugger::breakDebugger()
{
	__debugbreak();
}

void Debugger::reportEvent(const std::wstring& text, ...)
{
}

}
