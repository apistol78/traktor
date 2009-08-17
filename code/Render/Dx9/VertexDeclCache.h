#ifndef traktor_render_VertexDeclCache_H
#define traktor_render_VertexDeclCache_H

#include <vector>
#include "Core/Config.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{

class VertexDeclCache : public Unmanaged
{
public:
	VertexDeclCache(UnmanagedListener* listener, IDirect3DDevice9* d3dDevice);

	virtual ~VertexDeclCache();

	bool createDeclaration(const std::vector< VertexElement >& vertexElements, ComRef< IDirect3DVertexDeclaration9 >& outVertexDeclaration, DWORD& outVertexStride);

	void setDeclaration(IDirect3DVertexDeclaration9* d3dVertexDeclaration);

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	struct Declaration
	{
		std::vector< VertexElement > vertexElements;
		ComRef< IDirect3DVertexDeclaration9 > d3dVertexDeclaration;
		DWORD d3dVertexStride;
	};

	ComRef< IDirect3DDevice9 > m_d3dDevice;
	std::vector< Declaration > m_cache;
};

	}
}

#endif	// traktor_render_VertexDeclCache_H
