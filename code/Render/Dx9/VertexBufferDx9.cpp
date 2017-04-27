/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx9/VertexBufferDx9.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx9", VertexBufferDx9, VertexBuffer)

VertexBufferDx9::VertexBufferDx9(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
