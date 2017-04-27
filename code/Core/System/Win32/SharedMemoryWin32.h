/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SharedMemoryWin32_H
#define traktor_SharedMemoryWin32_H

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

/*! \brief Win32 process shared memory. */
class T_DLLCLASS SharedMemoryWin32 : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryWin32();

	virtual ~SharedMemoryWin32();

	bool create(const std::wstring& name, uint32_t size);

	virtual Ref< IStream > read(bool exclusive) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > write() T_OVERRIDE T_FINAL;

	virtual bool clear() T_OVERRIDE T_FINAL;

private:
	HANDLE m_hMap;
	void* m_ptr;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryWin32_H
