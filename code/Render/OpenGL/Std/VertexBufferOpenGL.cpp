/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Containers/IdAllocator.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/VertexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

static IdAllocator s_idAllocator(0, 4095);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferOpenGL", VertexBufferOpenGL, VertexBuffer)

VertexBufferOpenGL::VertexBufferOpenGL(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
,	m_id(0)
{
	m_id = s_idAllocator.alloc();
}

VertexBufferOpenGL::~VertexBufferOpenGL()
{
	s_idAllocator.free(m_id);
}

	}
}
