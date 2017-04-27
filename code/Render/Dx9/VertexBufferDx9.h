/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferDx9_H
#define traktor_render_VertexBufferDx9_H

#include "Render/VertexBuffer.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class VertexBufferDx9 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferDx9(uint32_t bufferSize);

	virtual bool activate(IDirect3DDevice9* d3dDevice) = 0;
};
	
	}
}

#endif	// traktor_render_VertexBufferDx9_H
