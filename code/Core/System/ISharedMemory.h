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

/*! Process shared memory.
*
* Simple wrapper for shared memory across several processes.
* Designed around a simple "single writer/multiple readers" pattern.
*/
class T_DLLCLASS ISharedMemory : public Object
{
	T_RTTI_CLASS;

public:
	/*! Acquire read pointer from memory.
	 *
	 * \param exclusive If exclusive access to memory is required.
	 * \return Pointer to readable shared memory, null if failed to acquire access.
	 */
	virtual const void* acquireReadPointer(bool exclusive = false) = 0;

	/*! Release read pointer to memory.
	 */
	virtual void releaseReadPointer() = 0;

	/*! Acquire write pointer to memory.
	 *
	 * \return Pointer to writable shared memory, null if failed to acquire access.
	 */
	virtual void* acquireWritePointer() = 0;

	/*! Release write pointer to memory.
	 */
	virtual void releaseWritePointer() = 0;
};

}

