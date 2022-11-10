/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Class/Any.h"
#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"

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
class T_DLLCLASS BoxedAlignedVector : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedAlignedVector() = default;

	explicit BoxedAlignedVector(uint32_t size);

	template < typename ItemType >
	BoxedAlignedVector(const AlignedVector< ItemType >& arr)
	{
		m_arr.resize(arr.size());
		for (uint32_t i = 0; i < arr.size(); ++i)
			m_arr[i] = CastAny< ItemType >::set(arr[i]);
	}

	void reserve(uint32_t capacity);

	void resize(uint32_t size);

	void clear();

	int32_t size() const;

	bool empty() const;

	void push_back(const Any& value);

	void pop_back();

	const Any& front();

	const Any& back();

	void set(int32_t index, const Any& value);

	Any get(int32_t index);

	virtual std::wstring toString() const override final;

	template < typename ItemType >
	AlignedVector< ItemType > unbox() const
	{
		AlignedVector< ItemType > arr(m_arr.size());
		for (uint32_t i = 0; i < m_arr.size(); ++i)
			arr[i] = CastAny< ItemType >::get(m_arr[i]);
		return arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	AlignedVector< Any > m_arr;
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny < AlignedVector< InnerType >, false >
{
	static std::wstring typeName() {
		return L"traktor.AlignedVector< InnerType >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAlignedVector >(value.getObjectUnsafe());
	}
    static Any set(const AlignedVector< InnerType >& value) {
        return Any::fromObject(new BoxedAlignedVector(value));
    }
    static AlignedVector< InnerType > get(const Any& value) {
		return static_cast< BoxedAlignedVector* >(value.getObject())->unbox< InnerType >();
    }
};

/*!
 * \ingroup Core
 */
template < typename InnerType >
struct CastAny < const AlignedVector< InnerType >&, false >
{
	static std::wstring typeName() {
		return L"const traktor.AlignedVector< InnerType >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedAlignedVector >(value.getObjectUnsafe());
	}
    static Any set(const AlignedVector< InnerType >& value) {
        return Any::fromObject(new BoxedAlignedVector(value));
    }
    static AlignedVector< InnerType > get(const Any& value) {
		return static_cast< BoxedAlignedVector* >(value.getObject())->unbox< InnerType >();
	}
};

}
