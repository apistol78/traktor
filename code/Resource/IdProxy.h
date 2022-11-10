/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

/*! Resource proxy with embedded identifier.
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
	IdProxy() = default;

	IdProxy(const IdProxy< ResourceType >& rs)
	:	m_id(rs.m_id)
	,	m_handle(rs.m_handle)
	{
	}

	explicit IdProxy(const Guid& id)
	:	m_id(id)
	{
	}

	explicit IdProxy(ResourceHandle* handle)
	:	m_handle(handle)
	{
	}

	explicit IdProxy(ResourceHandle* handle, const Guid& id)
	:	m_id(id)
	,	m_handle(handle)
	{
	}

	explicit IdProxy< ResourceType >(ResourceType* resource)
	:	m_handle(new ExplicitResourceHandle(resource))
	{
	}

	/*! Get resource id.
	 *
	 * \return Resource id.
	 */
	const Guid& getId() const
	{
		return m_id;
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
	void consume()
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

private:
	Guid m_id;
	Ref< ResourceHandle > m_handle;
	intptr_t m_tag = 0;
};

	}
}

