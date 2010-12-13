#ifndef traktor_ITypedObject_H
#define traktor_ITypedObject_H

#include "Core/Ref.h"
#include "Core/InplaceRef.h"
#include "Core/Meta/Traits.h"
#include "Core/Rtti/TypeInfo.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Rtti declaration and implementation macros.
 * \ingroup Core
 */
//@{

#define T_RTTI_CLASS											\
	public:														\
		static const traktor::TypeInfo& getClassTypeInfo();		\
		virtual const traktor::TypeInfo& getTypeInfo() const;	\
	private:													\
		static traktor::TypeInfo ms_typeInfo;
	
#define T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)									\
	const traktor::TypeInfo& CLASS::getClassTypeInfo() { return ms_typeInfo; }	\
	const traktor::TypeInfo& CLASS::getTypeInfo() const { return ms_typeInfo; }

#define T_IMPLEMENT_RTTI_CLASS_ROOT(ID, CLASS)	\
	traktor::TypeInfo CLASS::ms_typeInfo(		\
		ID,										\
		sizeof(CLASS),							\
		0,										\
		false,									\
		0,										\
		0										\
	);											\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_CLASS(ID, CLASS, SUPER)	\
	traktor::TypeInfo CLASS::ms_typeInfo(			\
		ID,											\
		sizeof(CLASS),								\
		0,											\
		false,										\
		&traktor::type_of< SUPER >(),				\
		0											\
	);												\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_FACTORY_CLASS_ROOT(ID, VERSION, CLASS)	\
	traktor::TypeInfo CLASS::ms_typeInfo(						\
		ID,														\
		sizeof(CLASS),											\
		VERSION,												\
		false,													\
		0,														\
		new traktor::InstanceFactory< CLASS >()					\
	);															\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_FACTORY_CLASS(ID, VERSION, CLASS, SUPER)	\
	traktor::TypeInfo CLASS::ms_typeInfo(							\
		ID,															\
		sizeof(CLASS),												\
		VERSION,													\
		false,														\
		&traktor::type_of< SUPER >(),								\
		new traktor::InstanceFactory< CLASS >()						\
	);																\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_EDIT_CLASS_ROOT(ID, VERSION, CLASS)	\
	traktor::TypeInfo CLASS::ms_typeInfo(						\
		ID,														\
		sizeof(CLASS),											\
		VERSION,												\
		true,													\
		0,														\
		new traktor::InstanceFactory< CLASS >()					\
	);															\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_EDIT_CLASS(ID, VERSION, CLASS, SUPER)	\
	traktor::TypeInfo CLASS::ms_typeInfo(						\
		ID,														\
		sizeof(CLASS),											\
		VERSION,												\
		true,													\
		&traktor::type_of< SUPER >(),							\
		new traktor::InstanceFactory< CLASS >()					\
	);															\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

//@}

/*! \brief RTTI object.
 * \ingroup Core
 */
class T_DLLCLASS ITypedObject : public IRefCount
{
public:
	virtual const TypeInfo& getTypeInfo() const = 0;
};

/*! \brief Get type of object.
 * \ingroup Core
 *
 * \param obj Object to get type of.
 */
inline const TypeInfo& type_of(const ITypedObject* obj)
{
	T_ASSERT (obj);
	return obj->getTypeInfo();
}

/*! \brief Return type name.
 * \ingroup Core
 *
 * \param o Object.
 * \return Type name.
 */
inline const wchar_t* type_name(const ITypedObject* obj)
{
	return obj ? obj->getTypeInfo().getName() : L"(null)";
}

/*! \brief Get type of class.
 * \ingroup Core
 */
template < typename T >
const TypeInfo& type_of()
{
	return T::getClassTypeInfo();
}

/*! \brief Check if type is identical.
 * \ingroup Core
 */
template < typename T >
bool is_type_a(const TypeInfo& type)
{
	return is_type_a(type_of< T >(), type);
}

/*! \brief Check if type is derived from a base type.
 * \ingroup Core
 */
template < typename T >
bool is_type_of(const TypeInfo& type)
{
	return is_type_of(type_of< T >(), type);
}

/*! \brief Return type difference.
 * \ingroup Core
 */
template < typename T >
uint32_t type_difference(const TypeInfo& type)
{
	return type_difference(type_of< T >(), type);
}

/*! \brief Return type difference.
 * \ingroup Core
 */
template < typename B, typename T >
uint32_t type_difference()
{
	return type_difference(type_of< B >(), type_of< T >());
}

/*! \brief Check if an object is of a certain type.
 * \ingroup Core
 */
template < typename T >
bool is_a(const ITypedObject* obj)
{
	if (!obj)
		return false;

	const TypeInfo* classTypeInfo = &IsPointer< T >::base_t::getClassTypeInfo();
	for (const TypeInfo* typeInfo = &obj->getTypeInfo(); typeInfo; typeInfo = typeInfo->getSuper())
	{
		if (typeInfo == classTypeInfo)
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
T dynamic_type_cast(ITypedObject* obj)
{
	return is_a< T >(obj) ? static_cast< T >(obj) : 0;
}

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T >
T dynamic_type_cast(const ITypedObject* obj)
{
	return is_a< T >(obj) ? static_cast< T >(obj) : 0;
}

/*! \brief Dynamic cast object.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value, null if object isn't of correct type.
 */
template < typename T, typename T0 >
Ref< typename IsPointer< T >::base_t > dynamic_type_cast(const Ref< T0 >& obj)
{
	return dynamic_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, typename T0 >
T checked_type_cast(T0* obj)
{
	T_ASSERT (!obj || is_a< T >(obj));
	return static_cast< T >(obj);
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
template < typename T, bool AllowNull, typename T0 >
T checked_type_cast(T0* obj)
{
	T_ASSERT ((AllowNull || obj) && is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, typename T0 >
T checked_type_cast(const T0* obj)
{
	T_ASSERT (!obj || is_a< T >(obj));
	return static_cast< T >(obj);
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
template < typename T, bool AllowNull, typename T0 >
T checked_type_cast(const T0* obj)
{
	T_ASSERT ((AllowNull || obj) && is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, typename T0 >
Ref< typename IsPointer< T >::base_t > checked_type_cast(const Ref< T0 >& obj)
{
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, bool AllowNull, typename T0 >
Ref< typename IsPointer< T >::base_t > checked_type_cast(const Ref< T0 >& obj)
{
	T_ASSERT ((AllowNull || obj) && is_a< T >(obj.ptr()));
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, typename T0 >
Ref< typename IsPointer< T >::base_t > checked_type_cast(const InplaceRef< T0 >& obj)
{
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

/*! \brief Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param o Object.
 * \return Casted value.
 */
template < typename T, bool AllowNull, typename T0 >
Ref< typename IsPointer< T >::base_t > checked_type_cast(const InplaceRef< T0 >& obj)
{
	T_ASSERT ((AllowNull || obj) && is_a< T >(obj.ptr()));
	return checked_type_cast< typename IsPointer< T >::base_t* >(obj.ptr());
}

}

#endif	// traktor_ITypedObject_H
