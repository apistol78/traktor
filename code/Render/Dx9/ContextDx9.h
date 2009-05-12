#ifndef traktor_render_ContextDx10_H
#define traktor_render_ContextDx10_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX9 context.
 * \ingroup DX9 Xbox360
 */
class ContextDx9 : public Object
{
	T_RTTI_CLASS(ContextDx9)

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

	/*! \brief Enqueue resource for deletion.
	 *
	 * \param callback Callback object, must be allocated through new operator.
	 */
	void deleteResource(DeleteCallback* callback);

	/*! \brief Delete pending resources. */
	void deleteResources();

	/*! \brief Helper method to release COM objects.
	 *
	 * \param InterfaceType Type of COM object interface.
	 * \param unk COM object.
	 */
	template < typename InterfaceType >
	void releaseComRef(ComRef< InterfaceType >& unk)
	{
		if (unk)
		{
			deleteResource(new ReleaseComObjectCallback(unk));
			unk.release();
		}
	}

	/*! \brief Helper method to release array of COM objects.
	 *
	 * \param InterfaceType Type of COM object interface.
	 * \param unks COM objects.
	 */
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
	Semaphore m_deleteResourcesLock;
	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextDx10_H
