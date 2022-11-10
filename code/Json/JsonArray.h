/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Any.h"
#include "Core/Containers/AlignedVector.h"
#include "Json/JsonNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

/*! JSON Array node
 * \ingroup JSON
 *
 * Represent an JSON array.
 */
class T_DLLCLASS JsonArray : public JsonNode
{
	T_RTTI_CLASS;

public:
	/*! Push value at the back of the array.
	 *
	 * \param value Value to push.
	 */
	void push(const Any& value);

	/*! Check if array is empty.
	 *
	 * \return True if array is empty.
	 */
	bool empty() const { return m_array.empty(); }

	/*! Get value at the front of the array.
	 *
	 * \return Value of first element.
	 */
	const Any& front() { return m_array.front(); }

	/*! Get value at the back of the array.
	 *
	 * \return Value of last element.
	 */
	const Any& back() { return m_array.back(); }

	/*! Get number of elements in the array.
	 *
	 * \return Number of elements.
	 */
	uint32_t size() const { return uint32_t(m_array.size()); }

	/*! Get value of element at a specified location.
	 *
	 * \param index Element index.
	 * \return Value of element at index.
	 */
	const Any& get(uint32_t index) { return m_array[index]; }

	/*! Get vector of all values.
	 *
	 * \return Vector of all values.
	 */
	const AlignedVector< Any >& get() const { return m_array; }

	virtual bool write(OutputStream& os) const override;

private:
	AlignedVector< Any > m_array;
};

	}
}

