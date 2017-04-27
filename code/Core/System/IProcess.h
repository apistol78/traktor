/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_IProcess_H
#define traktor_IProcess_H

#include "Core/Object.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief System process.
 * \ingroup Core
 *
 * System process wrapper; interface to
 * query system processes.
 */
class T_DLLCLASS IProcess
:	public Object
,	public IWaitable
{
	T_RTTI_CLASS;

public:
	enum Priority
	{
		Lowest,
		Below,
		Normal,
		Above,
		Highest
	};

	enum StdPipe
	{
		SpStdIn,
		SpStdOut,
		SpStdErr
	};

	enum SignalType
	{
		StCtrlC,
		StCtrlBreak
	};

	/*! \brief Set process scheduling priority.
	 *
	 * \param New process priority.
	 * \return True if priority changed.
	 */
	virtual bool setPriority(Priority priority) = 0;

	/*! \brief Get pipe stream.
	 *
	 * \return Exit code.
	 */
	virtual Ref< IStream > getPipeStream(StdPipe pipe) = 0;

	/*! \brief Send signal to process.
	 *
	 * \param signalType Type of signal \sa SignalType.
	 * \return True if signal reached process.
	 */
	virtual bool signal(SignalType signalType) = 0;

	/*! \brief Terminate process.
	 *
	 * \param exitCode Exit code of process.
	 * \return True if process being terminated.
	 */
	virtual bool terminate(int32_t exitCode) = 0;

	/*! \brief Get exit code returned by process.
	 *
	 * \return Exit code.
	 */
	virtual int32_t exitCode() const = 0;
};

}

#endif	// traktor_IProcess_H
