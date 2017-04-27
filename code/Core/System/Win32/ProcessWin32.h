/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ProcessWin32_H
#define traktor_ProcessWin32_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/System/IProcess.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Win32 system process.
 * \ingroup Core
 */
class T_DLLCLASS ProcessWin32 : public IProcess
{
	T_RTTI_CLASS;

public:
	ProcessWin32(
		HANDLE hProcess,
		DWORD dwProcessId,
		HANDLE hThread,
		HANDLE hStdInRead,
		HANDLE hStdInWrite,
		HANDLE hStdOutRead,
		HANDLE hStdOutWrite,
		HANDLE hStdErrRead,
		HANDLE hStdErrWrite
	);

	virtual ~ProcessWin32();

	virtual bool setPriority(Priority priority) T_OVERRIDE T_FINAL;

	virtual bool wait(int32_t timeout) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > getPipeStream(StdPipe pipe) T_OVERRIDE T_FINAL;

	virtual bool signal(SignalType signalType) T_OVERRIDE T_FINAL;

	virtual bool terminate(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t exitCode() const T_OVERRIDE T_FINAL;

private:
	HANDLE m_hProcess;
	DWORD m_dwProcessId;
	HANDLE m_hThread;
	HANDLE m_hStdInRead;
	HANDLE m_hStdInWrite;
	HANDLE m_hStdOutRead;
	HANDLE m_hStdOutWrite;
	HANDLE m_hStdErrRead;
	HANDLE m_hStdErrWrite;
	Ref< IStream > m_pipeStdOut;
	Ref< IStream > m_pipeStdErr;
};

}

#endif	// traktor_ProcessWin32_H
