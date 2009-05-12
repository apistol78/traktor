#ifndef traktor_render_IndexBufferDx9_H
#define traktor_render_IndexBufferDx9_H

#include "Render/IndexBuffer.h"
#include "Render/Dx9/Unmanaged.h"
#include "Core/Heap/Ref.h"
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

/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS IndexBufferDx9
:	public IndexBuffer
,	public Unmanaged
{
	T_RTTI_CLASS(IndexBufferDx9)

public:
	IndexBufferDx9(UnmanagedListener* unmanagedListener, ContextDx9* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDx9();

	bool create(IDirect3DDevice9* d3dDevice, bool dynamic);

	void activate(IDirect3DDevice9* d3dDevice);

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	static IndexBufferDx9* ms_activeIndexBuffer;

	Ref< ContextDx9 > m_context;
	ComRef< IDirect3DIndexBuffer9 > m_d3dIndexBuffer;
	D3DFORMAT m_d3dFormat;
	bool m_dynamic;
};
	
	}
}

#endif	// traktor_render_IndexBufferDx9_H
