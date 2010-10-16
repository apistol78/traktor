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

/*! \brief Is type const. */
template < typename Type >
struct IsConst
{
	enum { value = false };
};

/*! \brief Is type const, specialization. */
template < typename Type >
struct IsConst < const Type >
{
	enum { value = true };
};

//@}

}

#endif	// traktor_Traits_H
