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

namespace traktor
{
	namespace resource
	{

/*! Resource identity.
 * \ingroup Resource
 *
 * A resource identity is a unique identity of a
 * persistent resource along with the type of the resource asset.
 */
template < typename ResourceType >
class Id
{
public:
	Id() = default;

	Id(const Id& rh)
	:	m_id(rh.m_id)
	{
	}

	explicit Id(const Guid& id)
	:	m_id(id)
	{
	}

	explicit Id(const wchar_t* const id)
	:	m_id(Guid(id))
	{
	}

	bool isNull() const
	{
		return m_id.isNull();
	}

	bool isValid() const
	{
		return m_id.isValid();
	}

	bool operator < (const Id& rh) const
	{
		return m_id < rh.m_id;
	}

	bool operator > (const Id& rh) const
	{
		return m_id > rh.m_id;
	}

	bool operator == (const Id& rh) const
	{
		return m_id == rh.m_id;
	}

	bool operator != (const Id& rh) const
	{
		return m_id != rh.m_id;
	}

	operator bool () const
	{
		return !isNull() && isValid();
	}

	operator const Guid& () const
	{
		return m_id;
	}

private:
	Guid m_id;
};

	}
}

