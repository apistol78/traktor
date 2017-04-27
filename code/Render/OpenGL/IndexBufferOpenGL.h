/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferOpenGL_H
#define traktor_render_IndexBufferOpenGL_H

#include "Render/IndexBuffer.h"
#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class IndexBufferOpenGL : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferOpenGL(IndexType indexType, uint32_t bufferSize);

#if defined(T_OPENGL_STD)
	virtual void bind() = 0;

	virtual const GLvoid* getIndexData() const = 0;
#endif
};
	
	}
}

#endif	// traktor_render_IndexBufferOpenGL_H
