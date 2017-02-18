#ifndef traktor_ProcessEmscripten_H
#define traktor_ProcessEmscripten_H

#include <spawn.h>
#include "Core/System/IProcess.h"

namespace traktor
{

class ProcessEmscripten : public IProcess
{
	T_RTTI_CLASS;

public:
	virtual bool setPriority(Priority priority) T_OVERRIDE T_FINAL;
	
	virtual Ref< IStream > getPipeStream(StdPipe pipe) T_OVERRIDE T_FINAL;

	virtual bool signal(SignalType signalType) T_OVERRIDE T_FINAL;

	virtual int32_t exitCode() const T_OVERRIDE T_FINAL;

	virtual bool wait(int32_t timeout = -1) T_OVERRIDE T_FINAL;

private:
	friend class OS;

	pid_t m_pid;
	int32_t m_exitCode;

	ProcessEmscripten(pid_t pid);
};

}

#endif	// traktor_ProcessEmscripten_H
