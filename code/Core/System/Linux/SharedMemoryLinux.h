/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SharedMemoryLinux_H
#define traktor_SharedMemoryLinux_H

#include <string>
#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryLinux : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryLinux(const std::wstring& name, uint32_t size);

	virtual ~SharedMemoryLinux();

	virtual Ref< IStream > read(bool exclusive);

	virtual Ref< IStream > write();

	virtual bool clear();

private:
	std::wstring m_name;
	int m_fd;
	void* m_ptr;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryLinux_H
