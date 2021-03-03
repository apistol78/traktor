#pragma once

#include <spawn.h>
#include "Core/System/IProcess.h"

namespace traktor
{

class ProcessEmscripten : public IProcess
{
	T_RTTI_CLASS;

public:
	virtual bool setPriority(Priority priority) override final;

	virtual IStream* getPipeStream(StdPipe pipe) override final;

	virtual IStream* waitPipeStream(int32_t timeout) override final;

	virtual bool signal(SignalType signalType) override final;

	virtual int32_t exitCode() const override final;

	virtual bool wait(int32_t timeout = -1) override final;

private:
	friend class OS;

	pid_t m_pid;
	int32_t m_exitCode;

	ProcessEmscripten(pid_t pid);
};

}

