#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferOpenGL", IndexBufferOpenGL, IndexBuffer)

IndexBufferOpenGL::IndexBufferOpenGL(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
{
}

	}
}
