/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Range.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedRange : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedRange() = default;

	template < typename ItemType >
	BoxedRange(const Range< ItemType >& range)
	{
		m_min = CastAny< ItemType >::set(range.min);
		m_max = CastAny< ItemType >::set(range.max);
	}

	const Any& min() const { return m_min; }

	const Any& max() const { return m_max; }

	virtual std::wstring toString() const override final;

	template < typename ItemType >
	Range< ItemType > unbox() const
	{
		return Range< ItemType >(
			CastAny< ItemType >::get(m_min),
			CastAny< ItemType >::get(m_max)
		);
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Any m_min;
	Any m_max;
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny< Range< InnerType >, false >
{
	static std::wstring typeName() {
		return L"traktor.Range< InnerType >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return static_cast< BoxedRange* >(value.getObject())->unbox< InnerType >();
	}
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny< const Range< InnerType >&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Range< InnerType >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRange >(value.getObjectUnsafe());
	}
	static Any set(const Range< InnerType >& value) {
		return Any::fromObject(new BoxedRange(value));
	}
	static Range< InnerType > get(const Any& value) {
		return static_cast< BoxedRange* >(value.getObject())->unbox< InnerType >();
	}
};

}
