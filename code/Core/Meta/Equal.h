/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Equal_H
#define traktor_Equal_H

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief Type equal check. */
template <
	typename Type1,
	typename Type2
>
struct Equal
{
	enum { value = false };
};

/*! \brief Type equal check, matching type specialization. */
template <
	typename Type
>
struct Equal < Type, Type >
{
	enum { value = true };
};

//@}

}

#endif	// traktor_Equal_H
