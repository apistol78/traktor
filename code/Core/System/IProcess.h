/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
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

/*! System process.
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

	/*! Set process scheduling priority.
	 *
	 * \param New process priority.
	 * \return True if priority changed.
	 */
	virtual bool setPriority(Priority priority) = 0;

	/*! Get pipe stream.
	 *
	 * \return Pipe stream.
	 */
	virtual IStream* getPipeStream(StdPipe pipe) = 0;

	/*! Wait for a pipe stream which is ready to be read.
	 *
	 * \return Pipe stream.
	 */
	virtual IStream* waitPipeStream(int32_t timeout) = 0;

	/*! Send signal to process.
	 *
	 * \param signalType Type of signal \sa SignalType.
	 * \return True if signal reached process.
	 */
	virtual bool signal(SignalType signalType) = 0;

	/*! Terminate process.
	 *
	 * \param exitCode Exit code of process.
	 * \return True if process being terminated.
	 */
	virtual bool terminate(int32_t exitCode) = 0;

	/*! Get exit code returned by process.
	 *
	 * \return Exit code.
	 */
	virtual int32_t exitCode() const = 0;
};

}

