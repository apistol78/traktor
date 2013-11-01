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
	virtual Ref< IStream > getPipeStream(StdPipe pipe);
	
	virtual bool signal(SignalType signalType);
	
	virtual int32_t exitCode() const;
	
	virtual bool wait(int32_t timeout = -1);
	
private:
	friend class OS;
	
	pid_t m_pid;
	int32_t m_exitCode;
	
	ProcessOsX(pid_t pid);
};

}

#endif	// traktor_ProcessOsX_H
