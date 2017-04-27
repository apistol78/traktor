/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexDeclCache_H
#define traktor_render_VertexDeclCache_H

#include <vector>
#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Core/Thread/Semaphore.h"
#include "Render/VertexElement.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class VertexDeclCache : public Object
{
public:
	VertexDeclCache(IDirect3DDevice9* d3dDevice);

	bool createDeclaration(const std::vector< VertexElement >& vertexElements, ComRef< IDirect3DVertexDeclaration9 >& outVertexDeclaration, DWORD& outVertexStride);

	void setDeclaration(IDirect3DVertexDeclaration9* d3dVertexDeclaration);

	HRESULT lostDevice();

	HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	struct Declaration
	{
		std::vector< VertexElement > vertexElements;
		ComRef< IDirect3DVertexDeclaration9 > d3dVertexDeclaration;
		DWORD d3dVertexStride;
	};

	Semaphore m_lock;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	std::vector< Declaration > m_cache;
	IDirect3DVertexDeclaration9* m_currentVertexDeclaration;
};

	}
}

#endif	// traktor_render_VertexDeclCache_H
