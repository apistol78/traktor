#include "Render/OpenGL/ES/VertexBufferOpenGLES.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferOpenGLES", VertexBufferOpenGLES, VertexBuffer)

VertexBufferOpenGLES::VertexBufferOpenGLES(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
