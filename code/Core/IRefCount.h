/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include "Core/Config.h"

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
	virtual ~IRefCount() = default;

	virtual void addRef(void* owner) const = 0;

	virtual void release(void* owner) const = 0;
};

/*! Reference counted implementation.
 * \ingroup Core
 */
template < typename T >
class RefCountImpl : public T
{
public:
	RefCountImpl() = default;

	RefCountImpl(const RefCountImpl& object)
	{
		// Do not copy reference count.
	}

	RefCountImpl(RefCountImpl&& object) noexcept
	{
		// Do not move reference count.
	}

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

	RefCountImpl& operator = (const RefCountImpl&)
	{
		// Do not copy reference count.
		return *this;
	}

	RefCountImpl& operator = (RefCountImpl&&) noexcept
	{
		// Do not move reference count.
		return *this;
	}

protected:
	mutable std::atomic< int32_t > m_refCount = 0;
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
