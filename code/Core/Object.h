#ifndef traktor_Object_H
#define traktor_Object_H

#include "Core/Config.h"
#include "Core/Rtti/Type.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief Base class.
 *
 * This class is used as a basis for all classes which
 * needs to support RTTI and automatic memory management.
 */
class T_DLLCLASS Object
{
	T_RTTI_CLASS(Object);

public:
	Object();

	virtual ~Object();

	/*! \brief Check if object is managed by heap.
	 *
	 * \return True if object is managed by heap.
	 */
	inline bool isManaged() const { return m_managed; }

private:
	friend class Heap;

	bool m_managed;
};

template < typename T >
struct dereference_type
{
	enum { is_pointer = false };
	typedef T org;
};

template < typename T >
struct dereference_type < T* >
{
	enum { is_pointer = true };
	typedef T org;
};

/*! \brief Check if an object is of a certain type. */
template < typename T >
inline bool is_a(const Object* o)
{
	if (!o)
		return false;

	const Type* ot = &dereference_type< T >::org::getClassType();
	for (const Type* type = &o->getType(); type; type = type->getSuper())
	{
		if (type == ot)
			return true;
	}

	return false;
}

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T >
inline T dynamic_type_cast(Object* o)
{
	return is_a< T >(o) ? static_cast< T >(o) : 0;
}

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T >
inline T dynamic_type_cast(const Object* o)
{
	return is_a< T >(o) ? static_cast< T >(o) : 0;
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T >
inline T checked_type_cast(Object* o)
{
	T_ASSERT (!o || is_a< T >(o));
	return static_cast< T >(o);
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param AllowNull If object is allowed to be null.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, bool AllowNull >
inline T checked_type_cast(Object* o)
{
	T_ASSERT ((AllowNull || o) && is_a< T >(o));
	return static_cast< T >(o);
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T >
inline T checked_type_cast(const Object* o)
{
	T_ASSERT (!o || is_a< T >(o));
	return static_cast< T >(o);
}

/*! \brief Get type of object.
*
* \param object Object to get type of.
*/
inline const Type& type_of(const Object* object)
{
	T_ASSERT (object);
	return object->getType();
}

/*! \brief Return RTTI type name.
 *
 * \param o Object.
 * \return Type name.
 */
inline const wchar_t* type_name(const Object* o)
{
	return o ? o->getType().getName() : L"(null)";
}

//@}

}

#endif	// traktor_Object_H
