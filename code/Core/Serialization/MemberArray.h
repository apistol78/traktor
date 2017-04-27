/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberArray_H
#define traktor_MemberArray_H

#include <string>
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
class TypeInfo;
class ISerializer;

/*! \brief Array member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberArray
{
public:
	MemberArray(const wchar_t* const name, const Attribute* attributes);

	virtual ~MemberArray() {}

	/*! \brief Get name of member.
	 *
	 * \return Member's name.
	 */
	const wchar_t* const getName() const { return m_name; }

	/*! \brief Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	const Attribute* getAttributes() const { return m_attributes; }

	/*!
	 * Reserve size for X number of elements.
	 * \param size Known number of elements.
	 * \param capacity Estimated number of elements.
	 */
	virtual void reserve(size_t size, size_t capacity) const = 0;

	/*!
	 * Return size of the array in number of elements.
	 */
	virtual size_t size() const = 0;

	/*!
	 * Read next element from serializer.
	 */
	virtual void read(ISerializer& s) const = 0;

	/*!
	 * Write next element to serializer.
	 */
	virtual void write(ISerializer& s) const = 0;

	/*!
	* Insert default element, used by property list
	* to add new elements.
	*/
	virtual bool insert() const = 0;

private:
	const wchar_t* const m_name;
	const Attribute* m_attributes;
};

}

#endif	// traktor_MemberArray_H
