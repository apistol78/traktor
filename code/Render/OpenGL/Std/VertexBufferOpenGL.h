/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferOpenGL_H
#define traktor_render_VertexBufferOpenGL_H

#include "Render/VertexBuffer.h"
#include "Render/OpenGL/Std/Platform.h"

namespace traktor
{
	namespace render
	{

class RenderContextOpenGL;

/*!
 * \ingroup OGL
 */
class VertexBufferOpenGL : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferOpenGL(uint32_t bufferSize);

	virtual ~VertexBufferOpenGL();

	virtual void activate(RenderContextOpenGL* renderContext, const GLint* attributeLocs, uint32_t attributeHash) = 0;

protected:
	uint32_t m_id;
};
	
	}
}

#endif	// traktor_render_VertexBufferOpenGL_H
