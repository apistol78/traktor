/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/IRefCount.h"
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

/*! Rtti declaration and implementation macros.
 * \ingroup Core
 */
//@{

#define T_RTTI_CLASS													\
	public:																\
		static const traktor::TypeInfo& getClassTypeInfo();				\
		virtual const traktor::TypeInfo& getTypeInfo() const override;	\
	private:															\
		static traktor::TypeInfo ms_typeInfo;

#define T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)										\
	const traktor::TypeInfo& CLASS::getClassTypeInfo() { return ms_typeInfo; }		\
	const traktor::TypeInfo& CLASS::getTypeInfo() const { return ms_typeInfo; }

#define T_IMPLEMENT_RTTI_CLASS_ROOT(ID, CLASS)	\
	traktor::TypeInfo CLASS::ms_typeInfo(		\
		ID,										\
		sizeof(CLASS),							\
		0,										\
		false,									\
		nullptr,								\
		nullptr									\
	);											\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_CLASS(ID, CLASS, SUPER)	\
	traktor::TypeInfo CLASS::ms_typeInfo(			\
		ID,											\
		sizeof(CLASS),								\
		0,											\
		false,										\
		&traktor::type_of< SUPER >(),				\
		nullptr										\
	);												\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_VERSION_CLASS(ID, VERSION, CLASS, SUPER)	\
	traktor::TypeInfo CLASS::ms_typeInfo(							\
		ID,															\
		sizeof(CLASS),												\
		VERSION,													\
		false,														\
		&traktor::type_of< SUPER >(),								\
		nullptr														\
	);																\
	T_IMPLEMENT_RTTI_CLASS_COMMON(CLASS)

#define T_IMPLEMENT_RTTI_FACTORY_CLASS_ROOT(ID, VERSION, CLASS)		\
	traktor::TypeInfo CLASS::ms_typeInfo(							\
		ID,															\
		sizeof(CLASS),												\
		VERSION,													\
		false,														\
		nullptr,													\
		new traktor::InstanceFactory< CLASS >()						\
	);																\
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
		nullptr,												\
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

/*! RTTI object.
 * \ingroup Core
 */
class T_DLLCLASS ITypedObject : public IRefCount
{
public:
	static const TypeInfo& getClassTypeInfo();

	virtual const TypeInfo& getTypeInfo() const = 0;

private:
	static const traktor::TypeInfo ms_typeInfo;
};

/*! Get type of object.
 * \ingroup Core
 *
 * \param obj Object to get type of.
 */
inline const TypeInfo& type_of(const ITypedObject* obj)
{
	T_ASSERT(obj);
	return obj->getTypeInfo();
}

/*! Return type name.
 * \ingroup Core
 *
 * \param o Object.
 * \return Type name.
 */
inline const wchar_t* type_name(const ITypedObject* obj)
{
	return obj ? obj->getTypeInfo().getName() : L"(null)";
}

/*! Get type of class.
 * \ingroup Core
 */
template < typename T >
const TypeInfo& type_of()
{
	typedef typename IsPointer< typename IsReference< T >::base_t >::base_t tt;
	return tt::getClassTypeInfo();
}

/*! Return type name.
 * \ingroup Core
 *
 * \return Type name.
 */
template < typename T >
const wchar_t* type_name()
{
	return type_of< T >().getName();
}

/*! Create type info set from single type.
 * \ingroup Core
 */
template < typename ... Ts >
inline TypeInfoSet makeTypeInfoSet()
{
	return makeTypeInfoSet(type_of< Ts >() ... );
}

/*! Check if type is identical.
 * \ingroup Core
 */
template < typename T >
bool is_type_a(const TypeInfo& type)
{
	return is_type_a(type_of< T >(), type);
}

/*! Check if type is derived from a base type.
 * \ingroup Core
 */
template < typename T >
bool is_type_of(const TypeInfo& type)
{
	return is_type_of(type_of< T >(), type);
}

/*! Return type difference.
 * \ingroup Core
 */
template < typename T >
uint32_t type_difference(const TypeInfo& type)
{
	return type_difference(type_of< T >(), type);
}

/*! Return type difference.
 * \ingroup Core
 */
template < typename B, typename T >
uint32_t type_difference()
{
	return type_difference(type_of< B >(), type_of< T >());
}

/*! Check if an object is of a certain type.
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

/*! Dynamic cast object.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object, null if object isn't of correct type.
 */
template < typename T, typename T0 >
T dynamic_type_cast(T0* obj)
{
	return is_a< T >(obj) ? static_cast< T >(obj) : nullptr;
}

/*! Dynamic cast object.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object, null if object isn't of correct type.
 */
template < typename T, typename T0 >
T dynamic_type_cast(const T0* obj)
{
	return is_a< T >(obj) ? static_cast< T >(obj) : nullptr;
}

/*! Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, typename T0 >
T checked_type_cast(T0* obj)
{
	T_ASSERT(!obj || is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param AllowNull If object is allowed to be null.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, bool AllowNull, typename T0 >
T checked_type_cast(T0* obj)
{
	T_ASSERT((AllowNull && !obj) || is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, typename T0 >
T checked_type_cast(const T0* obj)
{
	T_ASSERT(!obj || is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! Safe cast object.
 *
 * The cast will assert if object is of incorrect type.
 *
 * \param T Cast to type.
 * \param AllowNull If object is allowed to be null.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, bool AllowNull, typename T0 >
T checked_type_cast(const T0* obj)
{
	T_ASSERT((AllowNull || obj) && is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! Safe cast object.
 *
 * The cast will cause system error if object is null or
 * of incorrect type.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, typename T0 >
T mandatory_non_null_type_cast(T0* obj)
{
	T_FATAL_ASSERT (obj && is_a< T >(obj));
	return static_cast< T >(obj);
}

/*! Safe cast object.
 *
 * The cast will cause system error if object is null or
 * of incorrect type.
 *
 * \param T Cast to type.
 * \param obj Object
 * \return Casted object.
 */
template < typename T, typename T0 >
T mandatory_non_null_type_cast(const T0* obj)
{
	T_FATAL_ASSERT (obj && is_a< T >(obj));
	return static_cast< T >(obj);
}

}
