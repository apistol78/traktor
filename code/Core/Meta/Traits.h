/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Traits_H
#define traktor_Traits_H

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief Is type a pointer. */
template < typename Type >
struct IsPointer
{
	typedef Type type_t;
	typedef Type base_t;

	enum { value = false };
};

/*! \brief Is type a pointer, pointer specialization. */
template < typename Type >
struct IsPointer < Type* >
{
	typedef Type* type_t;
	typedef Type base_t;

	enum { value = true };
};

/*! \brief Is type a pointer, const pointer specialization. */
template < typename Type >
struct IsPointer < const Type* >
{
	typedef const Type* type_t;
	typedef const Type base_t;

	enum { value = true };
};

/*! \brief Is type a reference. */
template < typename Type >
struct IsReference
{
	typedef Type type_t;
	typedef Type base_t;

	enum { value = false };
};

/*! \brief Is type a reference, reference specialization. */
template < typename Type >
struct IsReference < Type& >
{
	typedef Type& type_t;
	typedef Type base_t;

	enum { value = true };
};

/*! \brief Is type a reference, const reference specialization. */
template < typename Type >
struct IsReference < const Type& >
{
	typedef const Type& type_t;
	typedef const Type base_t;

	enum { value = true };
};

/*! \brief Is type const. */
template < typename Type >
struct IsConst
{
	typedef Type type_t;

	enum { value = false };
};

/*! \brief Is type const, specialization. */
template < typename Type >
struct IsConst < const Type >
{
	typedef Type type_t;

	enum { value = true };
};

//@}

}

#endif	// traktor_Traits_H
