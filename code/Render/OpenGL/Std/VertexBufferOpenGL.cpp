/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/VertexBufferOpenGL.h"

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
