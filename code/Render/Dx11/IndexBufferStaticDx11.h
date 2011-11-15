#ifndef traktor_render_IndexBufferStaticDx11_H
#define traktor_render_IndexBufferStaticDx11_H

#include "Core/Misc/AutoPtr.h"
#include "Render/Dx11/IndexBufferDx11.h"

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
class T_DLLCLASS IndexBufferStaticDx11 : public IndexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferStaticDx11 > create(ContextDx11* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferStaticDx11();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext);

private:
	AutoArrayPtr< uint8_t > m_data;

	IndexBufferStaticDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferStaticDx11_H
