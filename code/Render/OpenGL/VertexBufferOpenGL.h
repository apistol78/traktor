/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferOpenGL_H
#define traktor_render_VertexBufferOpenGL_H

#include "Render/VertexBuffer.h"
#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

class VertexElement;

/*!
 * \ingroup OGL
 */
class VertexBufferOpenGL : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferOpenGL(uint32_t bufferSize);

#if defined(T_OPENGL_STD)
	virtual void activate(const GLint* attributeLocs) = 0;
#endif
};
	
	}
}

#endif	// traktor_render_VertexBufferOpenGL_H
