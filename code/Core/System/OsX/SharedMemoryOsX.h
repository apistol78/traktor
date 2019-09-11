#pragma once

#include <string>
#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryOsX : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryOsX(uint32_t size);

	virtual ~SharedMemoryOsX();

	bool create(const std::wstring& name, uint32_t size);

	virtual const void* acquireReadPointer(bool exclusive) override final;

	virtual void releaseReadPointer() override final;

	virtual void* acquireWritePointer() override final;

	virtual void releaseWritePointer() override final;

private:
	std::wstring m_name;
	int m_fd;
	void* m_ptr;
	uint32_t m_size;
};

}

