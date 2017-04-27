/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferIBO_H
#define traktor_render_IndexBufferIBO_H

#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class IndexBufferIBO : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferIBO(ContextOpenGL* resourceContext, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferIBO();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void bind() T_OVERRIDE T_FINAL;

	virtual const GLvoid* getIndexData() const T_OVERRIDE T_FINAL;

private:
	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_name;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferIBO_H
