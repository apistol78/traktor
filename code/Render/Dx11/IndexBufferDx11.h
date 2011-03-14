#ifndef traktor_render_IndexBufferDx11_H
#define traktor_render_IndexBufferDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/IndexBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS IndexBufferDx11 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferDx11(ContextDx11* context, IndexType indexType, uint32_t bufferSize, ID3D11Buffer* d3dBuffer);

	virtual ~IndexBufferDx11();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	ID3D11Buffer* getD3D11Buffer() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Buffer > m_d3dBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferDx11_H
