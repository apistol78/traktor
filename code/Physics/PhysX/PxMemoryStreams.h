#ifndef traktor_physics_PxMemoryStreams_H
#define traktor_physics_PxMemoryStreams_H

#include <common/PxIO.h>

namespace traktor
{
	namespace physics
	{

class MemoryOutputStream : public physx::PxOutputStream
{
public:
	MemoryOutputStream();

	virtual ~MemoryOutputStream();

	physx::PxU32 write(const void* src, physx::PxU32 count);

	physx::PxU32 getSize() const { return m_size; }

	physx::PxU8* getData() const { return m_data; }

private:
	physx::PxU8* m_data;
	physx::PxU32 m_size;
	physx::PxU32 m_capacity;
};

class MemoryInputData : public physx::PxInputData
{
public:
	MemoryInputData(physx::PxU8* data, physx::PxU32 length);

	physx::PxU32 read(void* dest, physx::PxU32 count);

	physx::PxU32 getLength() const;

	void seek(physx::PxU32 pos);

	physx::PxU32 tell() const;

private:
	physx::PxU32 m_size;
	const physx::PxU8* m_data;
	physx::PxU32 m_pos;
};

	}
}

#endif	// traktor_physics_PxMemoryStreams_H
