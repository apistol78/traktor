#ifndef traktor_SharedMemoryOsX_H
#define traktor_SharedMemoryOsX_H

#include "Core/System/SharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryOsX : public SharedMemory
{
	T_RTTI_CLASS(SharedMemoryOsX)
	
public:
	SharedMemoryOsX(uint32_t size);
	
	virtual Stream* read(bool exclusive);
	
	virtual Stream* write();
	
private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryOsX_H
