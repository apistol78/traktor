#ifndef traktor_SharedMemoryOsX_H
#define traktor_SharedMemoryOsX_H

#include "Core/System/ISharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryOsX : public ISharedMemory
{
	T_RTTI_CLASS;
	
public:
	SharedMemoryOsX(uint32_t size);
	
	virtual Ref< IStream > read(bool exclusive);
	
	virtual Ref< IStream > write();
	
private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryOsX_H
