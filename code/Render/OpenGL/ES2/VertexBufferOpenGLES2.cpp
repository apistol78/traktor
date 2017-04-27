/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferOpenGLES2", VertexBufferOpenGLES2, VertexBufferOpenGL)

VertexBufferOpenGLES2::VertexBufferOpenGLES2(uint32_t bufferSize)
:	VertexBufferOpenGL(bufferSize)
{
}

	}
}
