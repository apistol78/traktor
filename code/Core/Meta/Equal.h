#pragma once

namespace traktor
{

/*! \ingroup Core */
//@{

/*! Type equal check. */
template <
	typename Type1,
	typename Type2
>
struct Equal
{
	enum { value = false };
};

/*! Type equal check, matching type specialization. */
template <
	typename Type
>
struct Equal < Type, Type >
{
	enum { value = true };
};

//@}

}

