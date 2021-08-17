#pragma once

#include "Render/IBufferView.h"
#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class BufferViewOpenGL : public IBufferView
{
	T_RTTI_CLASS;

public:
	BufferViewOpenGL() = default;

	explicit BufferViewOpenGL(GLuint buffer);

	GLuint getBuffer() const { return m_buffer; }

private:
	GLuint m_buffer = 0;
};

	}
}
