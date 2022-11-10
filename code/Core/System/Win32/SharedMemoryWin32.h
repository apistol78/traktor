/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/System/ISharedMemory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Win32 process shared memory. */
class T_DLLCLASS SharedMemoryWin32 : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryWin32();

	virtual ~SharedMemoryWin32();

	bool create(const std::wstring& name, uint32_t size);

	virtual const void* acquireReadPointer(bool exclusive) override final;

	virtual void releaseReadPointer() override final;

	virtual void* acquireWritePointer() override final;

	virtual void releaseWritePointer() override final;

private:
	HANDLE m_hMap;
	void* m_ptr;
	uint32_t m_size;
};

}

