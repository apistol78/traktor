#ifndef traktor_Process_H
#define traktor_Process_H

#include "Core/Object.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief System process.
 * \ingroup Core
 *
 * System process wrapper; interface to
 * query system processes.
 */
class T_DLLCLASS Process
:	public Object
,	public IWaitable
{
	T_RTTI_CLASS(Process)

public:
	/*! \brief Get exit code returned by process.
	 *
	 * \return Exit code.
	 */
	virtual int32_t exitCode() const = 0;
};

}

#endif	// traktor_Process_H
