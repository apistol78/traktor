#include "Render/Ps4/IndexBufferPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferPs4", IndexBufferPs4, IndexBuffer)

IndexBufferPs4::IndexBufferPs4(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
{
}

void IndexBufferPs4::prepare()
{
}

	}
}
