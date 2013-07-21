#ifndef traktor_SharedMemoryOsX_H
#define traktor_SharedMemoryOsX_H

#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryOsX : public ISharedMemory
{
	T_RTTI_CLASS;
	
public:
	SharedMemoryOsX(uint32_t size);
	
	virtual ~SharedMemoryOsX();
	
	virtual Ref< IStream > read(bool exclusive);
	
	virtual Ref< IStream > write();
	
	virtual bool clear();
	
private:
	uint32_t m_size;
	void* m_buffer;
};

}

#endif	// traktor_SharedMemoryOsX_H
