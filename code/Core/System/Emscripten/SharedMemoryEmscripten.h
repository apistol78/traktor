#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryEmscripten : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryEmscripten(uint32_t size);

	virtual const void* acquireReadPointer(bool exclusive) override final;

	virtual void releaseReadPointer() override final;

	virtual void* acquireWritePointer() override final;

	virtual void releaseWritePointer() override final;

private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

