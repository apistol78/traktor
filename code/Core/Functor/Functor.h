/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Functor_H
#define traktor_Functor_H

#include "Core/Ref.h"

#if defined(T_CXX11) && !defined(__PS3__)
#	include <functional>
#endif

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Functor base class.
 * \ingroup Core
 *
 * A functor is an object encapsulating a call to a function or a method on
 * a object. When creating a functor it's possible to "attach" argument values
 * as well.
 */
class T_DLLCLASS Functor : public RefCountImpl< IRefCount >
{
public:
	void* operator new (size_t size);

	void operator delete (void* ptr);

	virtual void operator () () = 0;
};

#if defined(T_CXX11) && !defined(__PS3__)

/*! \brief C++11 lambda functor.
 * \ingroup Core
 */
class LambdaFunctor : public Functor
{
public:
	LambdaFunctor(const std::function< void() >& fn)
	:	m_fn(fn)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		m_fn();
	}

private:
	std::function< void() > m_fn;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
inline Ref< LambdaFunctor > makeFunctor(const std::function< void() >& fn)
{
	return new LambdaFunctor(fn);
}

#endif	// T_CXX11

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T >
class Functor0 : public Functor
{
public:
	typedef void (T::*method_t)();

	Functor0(T* self, method_t method)
	:	m_self(self)
	,	m_method(method)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)();
	}

private:
	T* m_self;
	method_t m_method;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T >
inline Ref< Functor0< T > > makeFunctor(T* self, typename Functor0< T >::method_t method)
{
	return new Functor0< T >(self, method);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1 >
class Functor1 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1);

	Functor1(T* self, method_t method, P1 p1)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1 >
inline Ref< Functor1< T, P1 > > makeFunctor(T* self, typename Functor1< T, P1 >::method_t method, P1 p1)
{
	return new Functor1< T, P1 >(self, method, p1);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2 >
class Functor2 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1, P2 p2);

	Functor2(T* self, method_t method, P1 p1, P2 p2)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	,	m_p2(p2)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1, m_p2);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
	P2 m_p2;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2 >
inline Ref< Functor2< T, P1, P2 > > makeFunctor(T* self, typename Functor2< T, P1, P2 >::method_t method, P1 p1, P2 p2)
{
	return new Functor2< T, P1, P2 >(self, method, p1, p2);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3 >
class Functor3 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1, P2 p2, P3 p3);

	Functor3(T* self, method_t method, P1 p1, P2 p2, P3 p3)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1, m_p2, m_p3);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3 >
inline Ref< Functor3< T, P1, P2, P3 > > makeFunctor(T* self, typename Functor3< T, P1, P2, P3 >::method_t method, P1 p1, P2 p2, P3 p3)
{
	return new Functor3< T, P1, P2, P3 >(self, method, p1, p2, p3);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4 >
class Functor4 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1, P2 p2, P3 p3, P4 p4);

	Functor4(T* self, method_t method, P1 p1, P2 p2, P3 p3, P4 p4)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1, m_p2, m_p3, m_p4);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4 >
inline Ref< Functor4< T, P1, P2, P3, P4 > > makeFunctor(T* self, typename Functor4< T, P1, P2, P3, P4 >::method_t method, P1 p1, P2 p2, P3 p3, P4 p4)
{
	return new Functor4< T, P1, P2, P3, P4 >(self, method, p1, p2, p3, p4);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
class Functor5 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

	Functor5(T* self, method_t method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1, m_p2, m_p3, m_p4, m_p5);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5 >
inline Ref< Functor5< T, P1, P2, P3, P4, P5 > > makeFunctor(T* self, typename Functor5< T, P1, P2, P3, P4, P5 >::method_t method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	return new Functor5< T, P1, P2, P3, P4, P5 >(self, method, p1, p2, p3, p4, p5);
}

/*! \brief Class method functor.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
class Functor6 : public Functor
{
public:
	typedef void (T::*method_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

	Functor6(T* self, method_t method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	:	m_self(self)
	,	m_method(method)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	,	m_p6(p6)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(m_self->*m_method)(m_p1, m_p2, m_p3, m_p4, m_p5, m_p6);
	}

private:
	T* m_self;
	method_t m_method;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
	P6 m_p6;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
inline Ref< Functor6< T, P1, P2, P3, P4, P5, P6 > > makeFunctor(T* self, typename Functor6< T, P1, P2, P3, P4, P5, P6 >::method_t method, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
	return new Functor6< T, P1, P2, P3, P4, P5, P6 >(self, method, p1, p2, p3, p4, p5, p6);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
class StaticFunctor0 : public Functor
{
public:
	typedef void (*function_t)();

	StaticFunctor0(function_t function)
	:	m_function(function)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)();
	}

private:
	function_t m_function;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
inline Ref< StaticFunctor0 > makeStaticFunctor(StaticFunctor0::function_t function)
{
	return new StaticFunctor0(function);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1 >
class StaticFunctor1 : public Functor
{
public:
	typedef void (*function_t)(P1 p1);

	StaticFunctor1(function_t function, P1 p1)
	:	m_function(function)
	,	m_p1(p1)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1);
	}

private:
	function_t m_function;
	P1 m_p1;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1 >
inline Ref< StaticFunctor1< P1 > > makeStaticFunctor(typename StaticFunctor1< P1 >::function_t function, P1 p1)
{
	return new StaticFunctor1< P1 >(function, p1);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2 >
class StaticFunctor2 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2);

	StaticFunctor2(function_t function, P1 p1, P2 p2)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2 >
inline Ref< StaticFunctor2< P1, P2 > > makeStaticFunctor(typename StaticFunctor2< P1, P2 >::function_t function, P1 p1, P2 p2)
{
	return new StaticFunctor2< P1, P2 >(function, p1, p2);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3 >
class StaticFunctor3 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3);

	StaticFunctor3(function_t function, P1 p1, P2 p2, P3 p3)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3 >
inline Ref< StaticFunctor3< P1, P2, P3 > > makeStaticFunctor(typename StaticFunctor3< P1, P2, P3 >::function_t function, P1 p1, P2 p2, P3 p3)
{
	return new StaticFunctor3< P1, P2, P3 >(function, p1, p2, p3);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4 >
class StaticFunctor4 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3, P4 p4);

	StaticFunctor4(function_t function, P1 p1, P2 p2, P3 p3, P4 p4)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3, m_p4);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4 >
inline Ref< StaticFunctor4< P1, P2, P3, P4 > > makeStaticFunctor(typename StaticFunctor4< P1, P2, P3, P4 >::function_t function, P1 p1, P2 p2, P3 p3, P4 p4)
{
	return new StaticFunctor4< P1, P2, P3, P4 >(function, p1, p2, p3, p4);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5 >
class StaticFunctor5 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);

	StaticFunctor5(function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3, m_p4, m_p5);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5 >
inline Ref< StaticFunctor5< P1, P2, P3, P4, P5 > > makeStaticFunctor(typename StaticFunctor5< P1, P2, P3, P4, P5 >::function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
	return new StaticFunctor5< P1, P2, P3, P4, P5 >(function, p1, p2, p3, p4, p5);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
class StaticFunctor6 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6);

	StaticFunctor6(function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	,	m_p6(p6)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3, m_p4, m_p5, m_p6);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
	P6 m_p6;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
inline Ref< StaticFunctor6< P1, P2, P3, P4, P5, P6 > > makeStaticFunctor(typename StaticFunctor6< P1, P2, P3, P4, P5, P6 >::function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
{
	return new StaticFunctor6< P1, P2, P3, P4, P5, P6 >(function, p1, p2, p3, p4, p5, p6);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
class StaticFunctor7 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7);

	StaticFunctor7(function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	,	m_p6(p6)
	,	m_p7(p7)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3, m_p4, m_p5, m_p6, m_p7);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
	P6 m_p6;
	P7 m_p7;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7 >
inline Ref< StaticFunctor7< P1, P2, P3, P4, P5, P6, P7 > > makeStaticFunctor(typename StaticFunctor7< P1, P2, P3, P4, P5, P6, P7 >::function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
{
	return new StaticFunctor7< P1, P2, P3, P4, P5, P6, P7 >(function, p1, p2, p3, p4, p5, p6, p7);
}

/*! \brief Static function functor.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
class StaticFunctor8 : public Functor
{
public:
	typedef void (*function_t)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8);

	StaticFunctor8(function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	:	m_function(function)
	,	m_p1(p1)
	,	m_p2(p2)
	,	m_p3(p3)
	,	m_p4(p4)
	,	m_p5(p5)
	,	m_p6(p6)
	,	m_p7(p7)
	,	m_p8(p8)
	{
	}

	virtual void operator () () T_OVERRIDE T_FINAL
	{
		(*m_function)(m_p1, m_p2, m_p3, m_p4, m_p5, m_p6, m_p7, m_p8);
	}

private:
	function_t m_function;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
	P5 m_p5;
	P6 m_p6;
	P7 m_p7;
	P8 m_p8;
};

/*! \brief Create functor object.
 * \ingroup Core
 */
template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8 >
inline Ref< StaticFunctor8< P1, P2, P3, P4, P5, P6, P7, P8 > > makeStaticFunctor(typename StaticFunctor8< P1, P2, P3, P4, P5, P6, P7, P8 >::function_t function, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
{
	return new StaticFunctor8< P1, P2, P3, P4, P5, P6, P7, P8 >(function, p1, p2, p3, p4, p5, p6, p7, p8);
}

}

#endif	// traktor_Functor_H
