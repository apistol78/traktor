#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferOpenGL", VertexBufferOpenGL, VertexBuffer)

VertexBufferOpenGL::VertexBufferOpenGL(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
