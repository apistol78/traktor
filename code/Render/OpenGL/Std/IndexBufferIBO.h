#pragma once

#include "Render/IndexBuffer.h"
#include "Render/OpenGL/Std/Platform.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class IndexBufferIBO : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferIBO(ResourceContextOpenGL* resourceContext, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferIBO();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	void bind();

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_name;
	bool m_locked;
};

	}
}

