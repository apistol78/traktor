#ifndef traktor_render_ContextDx10_H
#define traktor_render_ContextDx10_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx10/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX10 context.
 * \ingroup DX10
 */
class ContextDx10 : public Object
{
	T_RTTI_CLASS(ContextDx10)

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

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

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
	Semaphore m_deleteResourcesLock;
	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextDx10_H
