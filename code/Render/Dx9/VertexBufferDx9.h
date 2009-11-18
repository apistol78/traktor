#ifndef traktor_render_VertexBufferDx9_H
#define traktor_render_VertexBufferDx9_H

#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Dx9/Unmanaged.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx9;
class RenderViewDx9;
class VertexDeclCache;

/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS VertexBufferDx9
:	public VertexBuffer
,	public Unmanaged
{
	T_RTTI_CLASS;

public:
	VertexBufferDx9(UnmanagedListener* unmanagedListener, ContextDx9* context, uint32_t bufferSize, VertexDeclCache* vertexDeclCache);

	virtual ~VertexBufferDx9();

	bool create(IDirect3DDevice9* d3dDevice, const std::vector< VertexElement >& vertexElements, bool dynamic);

	static void activate(IDirect3DDevice9* d3dDevice, VertexBufferDx9* vertexBuffer);

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	/*! \brief Force vertex buffer to dirty state.
	 *
	 * Next time any vertex buffer is activated the vertex buffer's state are
	 * unconditionally uploaded to the device.
	 */
	static void forceDirty();

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	static VertexBufferDx9* ms_activeVertexBuffer;

	Ref< ContextDx9 > m_context;
	VertexDeclCache* m_vertexDeclCache;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexDeclaration9 > m_d3dVertexDeclaration;
	DWORD m_d3dVertexStride;
	ComRef< IDirect3DVertexBuffer9 > m_d3dVertexBuffer;
	bool m_dynamic;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferDx9_H
