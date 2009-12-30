#ifndef traktor_render_IndexBufferPs3_H
#define traktor_render_IndexBufferPs3_H

#include "Render/IndexBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class LocalMemoryObject;

class T_DLLCLASS IndexBufferPs3 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferPs3(LocalMemoryObject* ibo, IndexType indexType, int bufferSize);

	virtual ~IndexBufferPs3();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	uint32_t getOffset() const;
	
private:
	LocalMemoryObject* m_ibo;
};
	
	}
}

#endif	// traktor_render_IndexBufferPs3_H
