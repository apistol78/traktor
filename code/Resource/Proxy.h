/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Resource/ExplicitResourceHandle.h"

namespace traktor::resource
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
	Proxy() = default;

	Proxy(const Proxy< ResourceType >& rs)
	:	m_handle(rs.m_handle)
	{
	}

	Proxy(Proxy< ResourceType >&& rs) noexcept
	{
		m_handle = std::move(rs.m_handle);
	}

	explicit Proxy(ResourceHandle* handle)
	:	m_handle(handle)
	{
	}

	explicit Proxy(ResourceType* resource)
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

	/*! Un-consume change; useful for forcing a consume again. */
	void unconsume()
	{
		m_tag = 0;
	}

	operator bool () const
	{
		return getResource() != nullptr;
	}

	operator ResourceType* () const
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

	bool operator == (const Proxy< ResourceType >& rh) const
	{
		return bool(getResource() == rh.getResource());
	}

	bool operator != (const Proxy< ResourceType >& rh) const
	{
		return bool(getResource() != rh.getResource());
	}

	Proxy< ResourceType >& operator = (const Proxy< ResourceType >& rh)
	{
		m_handle = rh.m_handle;
		return *this;
	}

	Proxy< ResourceType >& operator = (Proxy< ResourceType >&& rh) noexcept
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
