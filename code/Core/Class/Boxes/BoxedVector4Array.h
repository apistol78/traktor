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
#include "Core/Class/Boxed.h"
#include "Core/Class/CastAny.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedVector4;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedVector4Array : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedVector4Array() = default;

	explicit BoxedVector4Array(uint32_t size)
	:	m_arr(size)
	{
	}

	explicit BoxedVector4Array(const AlignedVector< Vector4 >& arr)
	:	m_arr(arr)
	{
	}

	void reserve(uint32_t capacity);

	void resize(uint32_t size);

	void clear();

	int32_t size() const;

	bool empty() const;

	void push_back(const BoxedVector4* value);

	void pop_back();

	Vector4 front();

	Vector4 back();

	void set(int32_t index, const BoxedVector4* value);

	Vector4 get(int32_t index);

	virtual std::wstring toString() const override final;

	const AlignedVector< Vector4 >& unbox() const
	{
		return m_arr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	AlignedVector< Vector4 > m_arr;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < AlignedVector< Vector4 >, false >
{
	static std::wstring typeName() {
		return L"traktor.AlignedVector< traktor.Vector4 >";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4Array >(value.getObjectUnsafe());
	}
	static Any set(const AlignedVector< Vector4 >& value) {
		return Any::fromObject(new BoxedVector4Array(value));
	}
	static const AlignedVector< Vector4 >& get(const Any& value) {
		return static_cast< BoxedVector4Array* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const AlignedVector< Vector4 >&, false >
{
	static std::wstring typeName() {
		return L"const traktor.AlignedVector< traktor.Vector4 >&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedVector4Array >(value.getObjectUnsafe());
	}
	static Any set(const AlignedVector< Vector4 >& value) {
		return Any::fromObject(new BoxedVector4Array(value));
	}
	static const AlignedVector< Vector4 >& get(const Any& value) {
		return static_cast< BoxedVector4Array* >(value.getObject())->unbox();
	}
};

}
