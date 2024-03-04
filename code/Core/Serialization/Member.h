/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <limits>
#include <string>
#include "Core/Config.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

/*! \ingroup Core */
//@{

class Attribute;

/*! Member serialization.
 *
 * Member class is a named descriptor of a member reference.
 * Serializer access named member through this object, both
 * when reading and writing to the members.
 */
template < typename T >
class Member
{
public:
	typedef T value_type;

	explicit Member(const wchar_t* const name, value_type& ref)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(nullptr)
	{
	}

	explicit Member(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(&attributes)
	{
	}

	virtual ~Member() {}

	/*! Get member name.
	 *
	 * \return Member name.
	 */
	const wchar_t* const getName() const { return m_name; }

	/*! Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	const Attribute* getAttributes() const { return m_attributes; }

	/*! Get member reference.
	 *
	 * \return Member reference.
	 */
	operator value_type& () const { return m_ref; }

	/*! Dereference member.
	 *
	 * \return Member reference.
	 */
	value_type& operator * () const { return m_ref; }

	/*! Dereference member.
	 *
	 * \return Member pointer.
	 */
	value_type* operator -> () const { return &m_ref; }

	/*! Assign value to member.
	 *
	 * \param value New member value.
	 */
	void operator = (const value_type& value) const { m_ref = value; }

private:
	const wchar_t* const m_name;
	value_type& m_ref;
	const Attribute* m_attributes;
};

/*! Member serialization.
 * \ingroup Core
 *
 * Member class is a named descriptor of a member reference.
 * Serializer access named member through this object, both
 * when reading and writing to the members.
 */
template < >
class Member < ISerializable* >
{
public:
	typedef Ref< ISerializable > value_type;

	explicit Member(const wchar_t* const name, value_type& ref)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(nullptr)
	{
	}

	explicit Member(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(&attributes)
	{
	}

	virtual ~Member() {}

	/*! Get member name.
	 *
	 * \return Member name.
	 */
	const wchar_t* const getName() const { return m_name; }

	/*! Get member value.
	 */
	ISerializable* getValue() const { return m_ref; }

	/*! Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	virtual const Attribute* getAttributes() const { return m_attributes; }

	/*! Get member reference.
	 *
	 * \return Member reference.
	 */
	operator value_type& () const { return m_ref; }

	/*! Dereference member.
	 *
	 * \return Member reference.
	 */
	value_type& operator * () const { return m_ref; }

	/*! Assign value to member.
	 *
	 * \param value New member value.
	 */
	void operator = (const value_type& value) const { m_ref = value; }

private:
	const wchar_t* const m_name;
	value_type& m_ref;
	const Attribute* m_attributes;
};

/*! Any-data member serialization.
 * \ingroup Core
 *
 * Specialized Member-class for data members as we want
 * additional information about the representation.
 */
template < >
class Member< void* >
{
public:
	explicit Member< void* >(const wchar_t* const name, void* blob, size_t& blobSize)
	:	m_name(name)
	,	m_blob(blob)
	,	m_blobSize(&blobSize)
	{
	}

	explicit Member< void* >(
		const wchar_t* const name,
		const std::function< size_t () >& fn_getSize,
		const std::function< bool (size_t) >& fn_setSize,
		const std::function< void* () >& fn_getPointer
	)
	:	m_name(name)
	,	m_fn_getSize(fn_getSize)
	,	m_fn_setSize(fn_setSize)
	,	m_fn_getPointer(fn_getPointer)
	{
	}

	/*! Get member name.
	 *
	 * \return Member name.
	 */
	const wchar_t* const getName() const { return m_name; }

	/*! Get pointer to binary blob.
	 *
	 * \return Pointer to binary blob.
	 */
	void* getBlob() const { return m_blob ? m_blob : m_fn_getPointer(); }

	/*! Get size of binary blob.
	 *
	 * \return Size of binary blob.
	 */
	size_t getBlobSize() const { return m_blobSize ? *m_blobSize : m_fn_getSize(); }

	/*! Set size of binary blob.
	 *
	 * \param blobSize Size of binary blob.
	 */
	bool setBlobSize(size_t blobSize) const
	{
		if (m_blobSize)
		{
			*m_blobSize = blobSize;
			return true;
		}
		else
			return m_fn_setSize(blobSize);
	}

private:
	const wchar_t* const m_name;

	// Simple access.
	void* m_blob = nullptr;
	size_t* m_blobSize = nullptr;

	// Lambda accessors.
	const std::function< size_t () > m_fn_getSize;
	const std::function< bool (size_t) > m_fn_setSize;
	const std::function< void* () > m_fn_getPointer;
};

/*! Obsolete member.
 * 
 * Instead of declaring dummy variables in each serialize method
 * use this wrapper instead.
 */
template < typename T >
class ObsoleteMember : public Member< T >
{
public:
	explicit ObsoleteMember(const wchar_t* const name)
	:	Member(name, m_dummy)
	{
	}

	explicit ObsoleteMember(const wchar_t* const name, const Attribute& attributes)
	:	Member(name, m_dummy, attributes)
	{
	}

private:
	value_type m_dummy;
};

//@}

}

