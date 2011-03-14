#ifndef traktor_render_ContextDx11_H
#define traktor_render_ContextDx11_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX11 context.
 * \ingroup DX11
 */
class ContextDx11 : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Garbage delete callback.
	 *
	 * Context keeps a queue for deletion callbacks
	 * to ensure resources are deleted properly (not used, same thread etc).
	 */
	struct DeleteCallback
	{
		virtual ~DeleteCallback() {}

		virtual void deleteResource() = 0;
	};

	/*! \brief Release COM object. */
	struct ReleaseComObjectCallback : public DeleteCallback
	{
		ComRef< IUnknown > m_unk;

		ReleaseComObjectCallback(IUnknown* unk);

		virtual void deleteResource();
	};

	/*! \brief Release COM objects. */
	struct ReleaseComObjectArrayCallback : public DeleteCallback
	{
		ComRefArray< IUnknown > m_unks;

		ReleaseComObjectArrayCallback(IUnknown** unks, size_t count);

		virtual void deleteResource();
	};

	ContextDx11(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext);

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

	ID3D11Device* getD3DDevice() { return m_d3dDevice; }

	ID3D11DeviceContext* getD3DDeviceContext() { return m_d3dDeviceContext; }

	template < typename InterfaceType >
	void releaseComRef(ComRef< InterfaceType >& unk)
	{
		if (unk)
		{
			deleteResource(new ReleaseComObjectCallback(unk));
			unk.release();
		}
	}

	template < typename InterfaceType >
	void releaseComRef(ComRefArray< InterfaceType >& unks)
	{
		if (!unks.empty())
		{
			deleteResource(new ReleaseComObjectArrayCallback((IUnknown**)unks.base(), unks.size()));
			unks.resize(0);
		}
	}

private:
	ComRef< ID3D11Device > m_d3dDevice;
	ComRef< ID3D11DeviceContext > m_d3dDeviceContext;
	Semaphore m_deleteResourcesLock;
	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextDx11_H
