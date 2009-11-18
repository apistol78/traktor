#ifndef traktor_IRefCount_H
#define traktor_IRefCount_H

#include "Core/Thread/Atomic.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Reference counted.
 * \ingroup Core
 */
class T_DLLCLASS IRefCount
{
public:
	virtual ~IRefCount() {}

	virtual void addRef() const = 0;

	virtual void release() const = 0;
};

/*! \brief Reference count value with atomic inc/dec operations.
 * \ingroup Core
 */
class T_DLLCLASS AtomicRefCount
{
public:
	AtomicRefCount()
	:	m_value(0)
	{
	}

	operator int32_t () const
	{
		return m_value;
	}

	int32_t operator ++ ()
	{
		return Atomic::increment(m_value);
	}

	int32_t operator -- ()
	{
		return Atomic::decrement(m_value);
	}

private:
	int32_t m_value;
};

/*! \brief Reference counted implementation.
 * \ingroup Core
 */
template < typename T >
class RefCountImpl : public T
{
public:
	virtual void addRef() const
	{
		++m_refCount;
	}

	virtual void release() const
	{
		T_ASSERT (m_refCount > 0);
		if (--m_refCount == 0)
			delete this;
	}

private:
	mutable AtomicRefCount m_refCount;
};

#if defined(T_TYPESAFE_REF)

/*! \brief Safe add reference template.
* \ingroup Core
*/
#	define T_SAFE_ADDREF(ptr) \
	if ((ptr)) \
		(ptr)->addRef();

/*! \brief Safe release template.
* \ingroup Core
*/
#	define T_SAFE_RELEASE(ptr) \
	if ((ptr)) \
		(ptr)->release();

#else

/*! \brief (Not as) Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_ADDREF(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->addRef();

/*! \brief (Not as) Safe release template.
* \ingroup Core
*/
#	define T_SAFE_RELEASE(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->release();

#endif

}

#endif	// traktor_IRefCount_H
