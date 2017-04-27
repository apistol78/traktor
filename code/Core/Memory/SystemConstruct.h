/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_SystemConstruct_H
#define traktor_SystemConstruct_H

#include "Core/Memory/Alloc.h"

namespace traktor
{

template < typename T >
T* allocConstruct()
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T();
}

template < typename T, typename A >
T* allocConstruct(A a)
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T(a);
}

template < typename T, typename A, typename B, typename C >
T* allocConstruct(A a, B b, C c)
{
	void* p = Alloc::acquireAlign(sizeof(T), alignOf< T >(), "global");
	return new (p) T(a, b, c);
}

template < typename T >
void freeDestruct(T* t)
{
	if (t)
	{
		t->~T();
		Alloc::freeAlign(t);
	}
}

}

#endif	// traktor_SystemConstruct_H
