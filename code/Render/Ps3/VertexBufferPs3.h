#ifndef traktor_render_VertexBufferPs3_H
#define traktor_render_VertexBufferPs3_H

#include <vector>
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS VertexBufferPs3 : public VertexBuffer
{
	T_RTTI_CLASS(VertexBufferPs3)

public:
	VertexBufferPs3(const std::vector< VertexElement >& vertexElements, void* ptr, uint32_t offset, int bufferSize);

	virtual ~VertexBufferPs3();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	void bind();

private:
	struct AttributeDesc
	{
		uint8_t size;
		uint8_t type;
		uint32_t offset;
	};

	void* m_ptr;
	uint32_t m_offset;
	uint8_t m_vertexStride;
	AttributeDesc m_attributeDesc[6];
};

	}
}

#endif	// traktor_render_VertexBufferPs3_H
