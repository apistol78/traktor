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
class ISerializer;

/*! Complex member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberComplex
{
public:
	explicit MemberComplex(const wchar_t* const name, bool compound);

	explicit MemberComplex(const wchar_t* const name, bool compound, const Attribute& attributes);

	virtual ~MemberComplex();

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

	/*! Check if member is compound.
	 *
	 * \return True if member is compound.
	 */
	bool getCompound() const { return m_compound; }

	virtual void serialize(ISerializer& s) const = 0;

private:
	const wchar_t* const m_name;
	bool m_compound;
	const Attribute* m_attributes;
};

}

