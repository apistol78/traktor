#ifndef traktor_render_Unmanaged_H
#define traktor_render_Unmanaged_H

#include <list>
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class Unmanaged;

/*!
 * \ingroup DX9 Xbox360
 */
struct UnmanagedListener
{
	virtual void addUnmanaged(Unmanaged* unmanaged) = 0;

	virtual void removeUnmanaged(Unmanaged* unmanaged) = 0;
};

/*!
 * \ingroup DX9 Xbox360
 */
class Unmanaged
{
public:
	Unmanaged(UnmanagedListener* listener)
	:	m_listener(listener)
	{
	}

protected:
	friend class RenderSystemWin32;
	friend class RenderSystemXbox360;

	void addToListener()
	{
		m_listener->addUnmanaged(this);
	}

	void removeFromListener()
	{
		m_listener->removeUnmanaged(this);
	}

	virtual HRESULT lostDevice() = 0;

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice) = 0;

private:
	UnmanagedListener* m_listener;
};

	}
}

#endif	// traktor_render_Unmanaged_H
