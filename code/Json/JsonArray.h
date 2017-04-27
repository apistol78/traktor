/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_json_JsonArray_H
#define traktor_json_JsonArray_H

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

/*! \brief JSON Array node
 * \ingroup JSON
 *
 * Represent an JSON array.
 */
class T_DLLCLASS JsonArray : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonArray();

	/*! \brief Push value at the back of the array.
	 *
	 * \param value Value to push.
	 */
	void push(const Any& value);

	/*! \brief Check if array is empty.
	 *
	 * \return True if array is empty.
	 */
	bool empty() const { return m_array.empty(); }

	/*! \brief Get value at the front of the array.
	 *
	 * \return Value of first element.
	 */
	const Any& front() { return m_array.front(); }

	/*! \brief Get value at the back of the array.
	 *
	 * \return Value of last element.
	 */
	const Any& back() { return m_array.back(); }

	/*! \brief Get number of elements in the array.
	 *
	 * \return Number of elements.
	 */
	uint32_t size() const { return uint32_t(m_array.size()); }

	/*! \brief Get value of element at a specified location.
	 *
	 * \param index Element index.
	 * \return Value of element at index.
	 */
	const Any& get(uint32_t index) { return m_array[index]; }

	/*! \brief Get vector of all values.
	 *
	 * \return Vector of all values.
	 */
	const AlignedVector< Any >& get() const { return m_array; }

	virtual bool write(OutputStream& os) const T_OVERRIDE;

private:
	AlignedVector< Any > m_array;
};

	}
}

#endif	// traktor_json_JsonArray_H
