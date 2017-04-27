/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_InvokeOnce_H
#define traktor_InvokeOnce_H

#include "Core/Thread/Atomic.h"

namespace traktor
{

/*! \brief Invoke method on pointer to object once then set pointer to null.
 * \ingroup Core
 */
// \{

template < typename Type, typename P0 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0), P0 p0)
{
	Type* ptr = Atomic::exchange< Type* >(ref, 0);
	if (ptr)
		(ptr->*M)(p0);
}

template < typename Type, typename P0, typename P1 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0, P1 p1), P0 p0, P1 p1)
{
	Type* ptr = Atomic::exchange< Type* >(ref, 0);
	if (ptr)
		(ptr->*M)(p0, p1);
}

template < typename Type, typename P0, typename P1, typename P2 >
void invokeOnce(Type*& ref, void (Type::*M)(P0 p0, P1 p1, P2 p2), P0 p0, P1 p1, P2 p2)
{
	Type* ptr = Atomic::exchange< Type* >(ref, 0);
	if (ptr)
		(ptr->*M)(p0, p1, p2);
}

// \}

}

#endif	// traktor_InvokeOnce_H