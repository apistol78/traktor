#ifndef traktor_resource_Proxy_H
#define traktor_resource_Proxy_H

#include "Core/Ref.h"
#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

/*! \brief Resource proxy.
 * \ingroup Resource
 *
 * A resource proxy is a reference wrapper with the
 * added functionality to resolve the object when
 * the reference is being dereferenced.
 */
template < typename ResourceType >
class Proxy
{
public:
	Proxy< ResourceType >()
	:	m_tag(0)
	{
	}

	Proxy< ResourceType >(const Proxy< ResourceType >& rs)
	:	m_handle(rs.m_handle)
	,	m_tag(0)
	{
	}

	explicit Proxy< ResourceType >(IResourceHandle* handle)
	:	m_handle(handle)
	,	m_tag(0)
	{
	}

	explicit Proxy< ResourceType >(ResourceType* resource)
	:	m_handle(new ExplicitResourceHandle(resource))
	,	m_tag(0)
	{
	}

	/*! \brief Get resource's handle.
	 *
	 * \return Resource handle.
	 */
	IResourceHandle* getHandle() const
	{
		return m_handle;
	}

	/*! \brief Replace resource handle.
	 *
	 * \param handle New resource handle.
	 */
	void replace(IResourceHandle* handle)
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
	Ref< IResourceHandle > m_handle;
	intptr_t m_tag;
};

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T, typename T0 >
typename IsPointer< T >::base_t* dynamic_type_cast(const Proxy< T0 >& obj)
{
	return dynamic_type_cast< typename IsPointer< T >::base_t* >(obj.getResource());
}

	}
}

#endif	// traktor_resource_Proxy_H
