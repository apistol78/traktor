/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	ContextDx11(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dDeviceContext,
		IDXGIFactory1* dxgiFactory,
		IDXGIOutput* dxgiOutput
	);

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

	Semaphore& getLock() { return m_lock; }

	ID3D11Device* getD3DDevice() { return m_d3dDevice; }

	ID3D11DeviceContext* getD3DDeviceContext() { return m_d3dDeviceContext; }

	IDXGIFactory1* getDXGIFactory() { return m_dxgiFactory; }

	IDXGIOutput* getDXGIOutput() { return m_dxgiOutput; }

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
	Semaphore m_lock;
	ComRef< ID3D11Device > m_d3dDevice;
	ComRef< ID3D11DeviceContext > m_d3dDeviceContext;
	ComRef< IDXGIFactory1 > m_dxgiFactory;
	ComRef< IDXGIOutput > m_dxgiOutput;
	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextDx11_H
