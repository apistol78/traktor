/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ProcessOsX_H
#define traktor_ProcessOsX_H

#include <spawn.h>
#include <sys/wait.h>
#include "Core/System/IProcess.h"

namespace traktor
{

class ProcessOsX : public IProcess
{
	T_RTTI_CLASS;
	
public:
	virtual ~ProcessOsX();

	virtual bool setPriority(Priority priority);

	virtual Ref< IStream > getPipeStream(StdPipe pipe);
	
	virtual bool signal(SignalType signalType);

	virtual bool terminate(int32_t exitCode);
	
	virtual int32_t exitCode() const;
	
	virtual bool wait(int32_t timeout = -1);
	
private:
	friend class OS;
	
	pid_t m_pid;
	posix_spawn_file_actions_t* m_fileActions;
	int m_childStdOut;
	int m_childStdErr;
	int32_t m_exitCode;

	ProcessOsX(pid_t pid, posix_spawn_file_actions_t* fileActions, int childStdOut, int childStdErr);
};

}

#endif	// traktor_ProcessOsX_H
