/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \ingroup Core */
//@{

/*! Aligned vector member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberAlignedVector : public MemberArray
{
public:
	typedef AlignedVector< ValueType > value_type;

	explicit MemberAlignedVector(const wchar_t* const name, value_type& ref)
		: MemberArray(name, nullptr)
		, m_ref(ref)
		, m_index(0)
	{
	}

	explicit MemberAlignedVector(const wchar_t* const name, value_type& ref, const Attribute& attributes)
		: MemberArray(name, &attributes)
		, m_ref(ref)
		, m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
		m_ref.reserve(capacity);
		m_ref.resize(size);
	}

	virtual size_t size() const override final
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s, size_t count) const override final
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (m_index < m_ref.size())
				s >> ValueMember(L"item", m_ref[m_index]);
			else
				s >> ValueMember(L"item", m_ref.push_back());

			++m_index;
		}
	}

	virtual void write(ISerializer& s, size_t count) const override final
	{
		for (size_t i = 0; i < count; ++i)
			s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const override final
	{
		m_ref.push_back(ValueType());
		return true;
	}

private:
	value_type& m_ref;
	mutable size_t m_index;
};

//@}

}
