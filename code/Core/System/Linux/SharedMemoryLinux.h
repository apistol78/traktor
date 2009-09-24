#ifndef traktor_SharedMemoryLinux_H
#define traktor_SharedMemoryLinux_H

#include "Core/System/SharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryLinux : public SharedMemory
{
	T_RTTI_CLASS(SharedMemoryLinux)
	
public:
	SharedMemoryLinux(uint32_t size);
	
	virtual Stream* read(bool exclusive);
	
	virtual Stream* write();
	
private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryLinux_H
