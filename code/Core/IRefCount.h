#pragma once

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

/*! Reference counted.
 * \ingroup Core
 */
class T_DLLCLASS IRefCount
{
public:
	virtual ~IRefCount() {}

	virtual void addRef(void* owner) const = 0;

	virtual void release(void* owner) const = 0;
};

/*! Reference count value with atomic inc/dec operations.
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

/*! Reference counted implementation.
 * \ingroup Core
 */
template < typename T >
class RefCountImpl : public T
{
public:
	virtual void addRef(void* owner) const override
	{
		++m_refCount;
	}

	virtual void release(void* owner) const override
	{
		T_ASSERT(m_refCount > 0);
		if (--m_refCount == 0)
			delete this;
	}

private:
	mutable AtomicRefCount m_refCount;
};

#if defined(T_TYPESAFE_REF)

/*! Safe add reference template.
* \ingroup Core
*/
#	define T_SAFE_ADDREF(ptr) \
	if ((ptr)) \
		(ptr)->addRef((void*)this);

/*! Safe release template.
* \ingroup Core
*/
#	define T_SAFE_RELEASE(ptr) \
	if ((ptr)) \
		(ptr)->release((void*)this);

/*! Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_ANONYMOUS_ADDREF(ptr) \
	if ((ptr)) \
		(ptr)->addRef(nullptr);

/*! Safe release template.
* \ingroup Core
*/
#	define T_SAFE_ANONYMOUS_RELEASE(ptr) \
	if ((ptr)) \
		(ptr)->release(nullptr);

/*! (Not as) Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_EXPLICIT_ADDREF(ptr, owner) \
	if ((ptr)) \
		(ptr)->addRef(owner);

/*! (Not as) Safe release template.
 * \ingroup Core
 */
#	define T_SAFE_EXPLICIT_RELEASE(ptr, owner) \
	if ((ptr)) \
		(ptr)->release(owner);

#else

/*! (Not as) Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_ADDREF(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->addRef((void*)this);

/*! (Not as) Safe release template.
* \ingroup Core
*/
#	define T_SAFE_RELEASE(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->release((void*)this);

/*! (Not as) Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_ANONYMOUS_ADDREF(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->addRef(nullptr);

/*! (Not as) Safe release template.
* \ingroup Core
*/
#	define T_SAFE_ANONYMOUS_RELEASE(ptr) \
	if ((ptr)) \
		((IRefCount*)(ptr))->release(nullptr);

/*! (Not as) Safe add reference template.
 * \ingroup Core
 */
#	define T_SAFE_EXPLICIT_ADDREF(ptr, owner) \
	if ((ptr)) \
		((IRefCount*)(ptr))->addRef(owner);

/*! (Not as) Safe release template.
 * \ingroup Core
 */
#	define T_SAFE_EXPLICIT_RELEASE(ptr, owner) \
	if ((ptr)) \
		((IRefCount*)(ptr))->release(owner);

#endif

}
