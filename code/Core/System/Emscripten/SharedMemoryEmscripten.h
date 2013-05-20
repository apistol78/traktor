#ifndef traktor_SharedMemoryEmscripten_H
#define traktor_SharedMemoryEmscripten_H

#include "Core/System/ISharedMemory.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class SharedMemoryEmscripten : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryEmscripten(uint32_t size);

	virtual Ref< IStream > read(bool exclusive);

	virtual Ref< IStream > write();

private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryEmscripten_H
