/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <windows.h>
#include <tchar.h>
#include <csignal>
#include <set>
#include <sstream>
#include "Core/Debug/CallStack.h"
#include "Core/Debug/Debugger.h"
#include "Core/Debug/Win32/Resource.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

enum EndCode
{
	EcBreak,
	EcAbort,
	EcIgnore,
	EcIgnoreAlways
};

struct DialogParams
{
	std::string expression;
	std::string file;
	int line;
	std::wstring message;
};

std::set< std::string > s_ignored;

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
			EndDialog(hWnd, EcBreak);
			return TRUE;

		case ID_ABORT:
			EndDialog(hWnd, EcAbort);
			return TRUE;

		case ID_IGNORE:
			{
				// If user hold shift then we assume "Ignore always".
				bool always = bool(GetAsyncKeyState(VK_SHIFT) != 0);
				EndDialog(hWnd, always ? EcIgnoreAlways : EcIgnore);
			}
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

void Debugger::assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message)
{
#if !defined(T_STATIC)
#   if defined(_DEBUG)
	HINSTANCE hInstance = GetModuleHandle(_T("Traktor.Core_d.dll"));
#   else
	HINSTANCE hInstance = GetModuleHandle(_T("Traktor.Core.dll"));
#   endif
#else
	HINSTANCE hInstance = GetModuleHandle(NULL);
#endif

	std::stringstream fileId;
	fileId << file << ":" << line;

	// Have this assert been ignored forever then we just keep going.
	if (s_ignored.find(fileId.str()) != s_ignored.end())
		return;

	// Show callstack if no message was given.
	std::wstring msg = message ? message : L"";
	if (msg.empty())
	{
		std::wstringstream cs;

		void* callstack[8];
		const uint32_t depth = getCallStack(8, callstack, 1);

		for (int32_t i = 0; i < depth; ++i)
		{
			std::wstring symbol;
			getSymbolFromAddress(callstack[i], symbol);

			std::wstring fileName;
			int32_t line;
			getSourceFromAddress(callstack[i], fileName, line);

			cs << symbol << L"    " << fileName << L"(" << line << L")" << std::endl;
		}

		msg = cs.str();
	}

	DialogParams params = { expression, file, line, msg.c_str() };

	INT_PTR result = DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_ASSERT_DIALOG),
		NULL,
		DialogProc,
		reinterpret_cast< LPARAM >(&params)
	);

	if (result == EcBreak)
		breakDebugger();
	else if (result == EcAbort)
	{
		raise(SIGABRT);
		exit(3);
	}
	else if (result == EcIgnoreAlways)
		s_ignored.insert(fileId.str());
}

bool Debugger::isDebuggerAttached() const
{
	return IsDebuggerPresent() != FALSE;
}

void Debugger::breakDebugger()
{
#if defined(_MSC_VER)
	__debugbreak();
#endif
}

void Debugger::reportEvent(const wchar_t* const text, ...)
{
}

}
