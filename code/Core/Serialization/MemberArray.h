#ifndef traktor_MemberArray_H
#define traktor_MemberArray_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Type;
class Serializer;

/*! \brief Array member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberArray
{
public:
	MemberArray(const std::wstring& name);

	const std::wstring& getName() const;

	/*!
	 * Return element type if available.
	 */
	virtual const Type* getType() const;

	/*!
	 * Reserve size for X number of elements.
	 * \note
	 *       size might not be set to the actual size, should only be
	 *       considered to be a hint.
	 */
	virtual void reserve(size_t size) const = 0;

	/*!
	 * Return size of the array in number of elements.
	 */
	virtual size_t size() const = 0;

	/*!
	 * Read element from serializer. Add to last position
	 * in array.
	 */
	virtual bool read(Serializer& s) const = 0;

	/*!
	 * Write element to serializer.
	 */
	virtual bool write(Serializer& s, size_t index) const = 0;

	/*!
	* Insert default element, used by property list
	* to add new elements.
	*/
	virtual bool insert() const = 0;

	/*!
	 * Serialize method, just dispatches to read or write
	 * depending on serializer.
	 */
	virtual bool serialize(Serializer& s, size_t index) const;

private:
	std::wstring m_name;
};

}

#endif	// traktor_MemberArray_H
