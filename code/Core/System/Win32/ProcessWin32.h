/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/System/IProcess.h"
#include "Core/Thread/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Win32 system process.
 * \ingroup Core
 */
class T_DLLCLASS ProcessWin32 : public IProcess
{
	T_RTTI_CLASS;

public:
	explicit ProcessWin32(
		HANDLE hProcess,
		DWORD dwProcessId,
		HANDLE hThread,
		HANDLE hStdInWrite,
		HANDLE hStdOutRead,
		HANDLE hStdErrRead
	);

	virtual ~ProcessWin32();

	virtual bool setPriority(Priority priority) override final;

	virtual bool wait(int32_t timeout) override final;

	virtual IStream* getPipeStream(StdPipe pipe) override final;

	virtual WaitPipeResult waitPipeStream(int32_t timeout, Ref< IStream >& outPipe) override final;

	virtual bool signal(SignalType signalType) override final;

	virtual bool terminate(int32_t exitCode) override final;

	virtual int32_t exitCode() const override final;

private:
	HANDLE m_hProcess;
	DWORD m_dwProcessId;
	HANDLE m_hThread;
	HANDLE m_hStdInWrite;
	HANDLE m_hStdOutRead;
	HANDLE m_hStdErrRead;
	Ref< IStream > m_pipeStdOut;
	Ref< IStream > m_pipeStdErr;
	Event m_pipeEvent;
};

}

