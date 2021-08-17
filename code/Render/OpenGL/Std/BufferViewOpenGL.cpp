#include "Render/OpenGL/Std/BufferViewOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferViewOpenGL", BufferViewOpenGL, IBufferView)

BufferViewOpenGL::BufferViewOpenGL(GLuint buffer)
:	m_buffer(buffer)
{
}

	}
}