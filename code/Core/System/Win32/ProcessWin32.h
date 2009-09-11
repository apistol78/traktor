#ifndef traktor_ProcessWin32_H
#define traktor_ProcessWin32_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/System/Process.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Win32 system process.
 * \ingroup Core
 */
class T_DLLCLASS ProcessWin32 : public Process
{
	T_RTTI_CLASS(Process)

public:
	ProcessWin32(const PROCESS_INFORMATION& pi);

	virtual ~ProcessWin32();

	virtual bool wait(int32_t timeout);

	virtual int32_t exitCode() const;

private:
	PROCESS_INFORMATION m_pi;
};

}

#endif	// traktor_ProcessWin32_H
