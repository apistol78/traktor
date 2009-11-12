#ifndef traktor_GcNew_H
#define traktor_GcNew_H

#include "Core/Heap/Heap.h"
#include "Core/Heap/Ref.h"

namespace traktor
{

/*! \ingroup Core */
//@{

#if defined (_MSC_VER)
#	pragma warning( disable : 4244 )	// Conversion from 'int' to 'float', possible loss of data
#endif

template < typename T >
T_GC_NEW_TYPE(T) gc_new()
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T;
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1 >
T_GC_NEW_TYPE(T) gc_new(P1 p1)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6, p7);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6, p7, p8);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6, p7, p8, p9);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	Heap::postConstructor(ptr);
	return obj;
}

template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10, typename P11 >
T_GC_NEW_TYPE(T) gc_new(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11)
{
	void* ptr = Heap::preConstructor(sizeof(T), alignOf< T >());
	T_GC_NEW_TYPE(T) obj = new(ptr) T (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
	Heap::postConstructor(ptr);
	return obj;
}

#if defined (_MSC_VER)
#	pragma warning( default : 4244 )
#endif

template < typename T >
class ReferenceWrapper
{
public:
	explicit ReferenceWrapper(T& v) : _v(v) {}

	operator T& () const { return _v; }

private:
	T& _v;
};

template < typename T > ReferenceWrapper< T > ref(T& t)
{
	return ReferenceWrapper< T >(t);
}

template < typename T > ReferenceWrapper< T const > cref(T const& t)
{
	return ReferenceWrapper< T const >(t);
}

//@}

}

#endif	// traktor_GcNew_H
