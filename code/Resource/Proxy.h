#pragma once

#include "Core/Ref.h"
#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

/*! Resource proxy.
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
	Proxy< ResourceType >() = default;

	Proxy< ResourceType >(const Proxy< ResourceType >& rs)
	:	m_handle(rs.m_handle)
	{
	}

	Proxy< ResourceType >(Proxy< ResourceType >&& rs)
	{
		m_handle = std::move(rs.m_handle);
	}

	explicit Proxy< ResourceType >(ResourceHandle* handle)
	:	m_handle(handle)
	{
	}

	explicit Proxy< ResourceType >(ResourceType* resource)
	:	m_handle(new ExplicitResourceHandle(resource))
	{
	}

	/*! Get resource's handle.
	 *
	 * \return Resource handle.
	 */
	ResourceHandle* getHandle() const
	{
		return m_handle;
	}

	/*! Replace resource handle.
	 *
	 * \param handle New resource handle.
	 */
	void replace(ResourceHandle* handle)
	{
		m_handle = handle;
	}

	/*! Get resource.
	 *
	 * \return Resource.
	 */
	ResourceType* getResource() const
	{
		return checked_type_cast< ResourceType*, true >(m_handle ? m_handle->get() : nullptr);
	}

	/*! Clear proxy; becoming completely unbound. */
	void clear()
	{
		m_handle = nullptr;
	}

	/*! Return true if resource has been changed. */
	bool changed() const
	{
		return (intptr_t)getResource() != m_tag;
	}

	/*! Consume change; changed method will return false until next change. */
	void consume() const
	{
		m_tag = (intptr_t)getResource();
	}

	operator bool () const
	{
		return getResource() != nullptr;
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

	Proxy< ResourceType >& operator = (const Proxy< ResourceType >& rh)
	{
		m_handle = rh.m_handle;
		return *this;
	}

	Proxy< ResourceType >& operator = (Proxy< ResourceType >&& rh)
	{
		m_handle = std::move(rh.m_handle);
		return *this;
	}

private:
	Ref< ResourceHandle > m_handle;
	mutable intptr_t m_tag = 0;
};

/*! Dynamic cast object.
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

