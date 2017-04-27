/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SharedMemoryPNaCl_H
#define traktor_SharedMemoryPNaCl_H

#include "Core/System/ISharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryPNaCl : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryPNaCl(uint32_t size);

	virtual Ref< IStream > read(bool exclusive);

	virtual Ref< IStream > write();

	virtual bool clear();

private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryPNaCl_H
