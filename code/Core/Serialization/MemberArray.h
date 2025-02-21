/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Attribute;
class TypeInfo;
class ISerializer;

/*! Array member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberArray
{
public:
	explicit MemberArray(const wchar_t* const name, const Attribute* attributes);

	virtual ~MemberArray() {}

	/*! Get name of member.
	 *
	 * \return Member's name.
	 */
	const wchar_t* const getName() const { return m_name; }

	/*! Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	const Attribute* getAttributes() const { return m_attributes; }

	/*! Reserve size for X number of elements.
	 *
	 * \param size Known number of elements.
	 * \param capacity Estimated number of elements.
	 */
	virtual void reserve(size_t size, size_t capacity) const = 0;

	/*! Return size of the array in number of elements. */
	virtual size_t size() const = 0;

	/*! Read next element from serializer. */
	virtual void read(ISerializer& s, size_t count) const = 0;

	/*! Write next element to serializer. */
	virtual void write(ISerializer& s, size_t count) const = 0;

	/*! Insert default element, used by property list to add new elements. */
	virtual bool insert() const = 0;

protected:
	/*! Set attributes member. */
	void setAttributes(const Attribute* attributes);

private:
	const wchar_t* const m_name;
	const Attribute* m_attributes;
};

}
