#ifndef traktor_SharedMemoryEmscripten_H
#define traktor_SharedMemoryEmscripten_H

#include "Core/Misc/AutoPtr.h"
#include "Core/System/ISharedMemory.h"

namespace traktor
{

class SharedMemoryEmscripten : public ISharedMemory
{
	T_RTTI_CLASS;

public:
	SharedMemoryEmscripten(uint32_t size);

	virtual Ref< IStream > read(bool exclusive) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > write() T_OVERRIDE T_FINAL;

	virtual bool clear() T_OVERRIDE T_FINAL;

private:
	AutoArrayPtr< uint8_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_SharedMemoryEmscripten_H
