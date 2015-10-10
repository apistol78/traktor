#ifndef traktor_resource_IdProxy_H
#define traktor_resource_IdProxy_H

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

/*! \brief Resource proxy with embedded identifier.
 * \ingroup Resource
 *
 * A resource proxy is a reference wrapper with the
 * added functionality to resolve the object when
 * the reference is being dereferenced.
 *
 * \note
 * Try not to use this proxy as it will probably
 * pollute code. This is mainly for legacy purposes
 * as a stepping stone for transition larger designs
 * to Id/Proxy separation.
 */
template < typename ResourceType >
class IdProxy
{
public:
	IdProxy()
	:	m_tag(0)
	{
	}

	IdProxy(const IdProxy< ResourceType >& rs)
	:	m_handle(rs.m_handle)
	,	m_id(rs.m_id)
	,	m_tag(0)
	{
	}

	explicit IdProxy(const Guid& id)
	:	m_id(id)
	,	m_tag(0)
	{
	}

	explicit IdProxy(ResourceHandle* handle)
	:	m_handle(handle)
	,	m_tag(0)
	{
	}

	explicit IdProxy(ResourceHandle* handle, const Guid& id)
	:	m_handle(handle)
	,	m_id(id)
	,	m_tag(0)
	{
	}

	explicit IdProxy< ResourceType >(ResourceType* resource)
	:	m_handle(new ExplicitResourceHandle(resource))
	,	m_tag(0)
	{
	}

	/*! \brief Get resource id.
	 *
	 * \return Resource id.
	 */
	const Guid& getId() const
	{
		return m_id;
	}

	/*! \brief Get resource's handle.
	 *
	 * \return Resource handle.
	 */
	ResourceHandle* getHandle() const
	{
		return m_handle;
	}

	/*! \brief Replace resource handle.
	 *
	 * \param handle New resource handle.
	 */
	void replace(ResourceHandle* handle)
	{
		m_handle = handle;
	}

	/*! \brief Get resource.
	 *
	 * \return Resource.
	 */
	ResourceType* getResource() const
	{
		return checked_type_cast< ResourceType*, true >(m_handle ? m_handle->get() : 0);
	}

	/*! \brief Clear proxy; becoming completely unbound. */
	void clear()
	{
		m_handle = 0;
	}

	/*! \brief Return true if resource has been changed. */
	bool changed() const
	{
		return intptr_t(getResource()) != m_tag;
	}

	/*! \brief Consume change; changed method will return false until next change. */
	void consume()
	{
		m_tag = intptr_t(getResource());
	}

	operator bool () const
	{
		return getResource() != 0;
	}

	operator ResourceType* () const
	{
		return getResource();
	}

	operator const ResourceType* () const
	{
		return getResource();
	}

	ResourceType* operator -> ()
	{
		return getResource();
	}

	const ResourceType* operator -> () const
	{
		return getResource();
	}

	bool operator == (const ResourceType* rs) const
	{
		return bool(getResource() == rs);
	}

	bool operator != (const ResourceType* rs) const
	{
		return bool(getResource() != rs);
	}

private:
	Guid m_id;
	Ref< ResourceHandle > m_handle;
	intptr_t m_tag;
};

	}
}

#endif	// traktor_resource_IdProxy_H
