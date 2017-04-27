/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferOpenGLES2_H
#define traktor_render_VertexBufferOpenGLES2_H

#include "Render/OpenGL/VertexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class StateCache;

/*!
 * \ingroup OGL
 */
class VertexBufferOpenGLES2 : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferOpenGLES2(uint32_t bufferSize);

	virtual void activate(StateCache* stateCache) = 0;
};

	}
}

#endif	// traktor_render_VertexBufferOpenGLES2_H
