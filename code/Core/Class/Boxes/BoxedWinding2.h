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
#include "Core/Math/Winding2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class BoxedVector2;

/*!
 * \ingroup Core
 */
class T_DLLCLASS BoxedWinding2 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedWinding2() = default;

	explicit BoxedWinding2(const Winding2& value);

	void clear();

	void push(const BoxedVector2* pt);

	bool inside(const BoxedVector2* pt) const;

	Vector2 closest(const BoxedVector2* pt) const;

	void reserve(uint32_t size);

	void resize(uint32_t size);

	uint32_t size() const;

	bool empty() const;

	Vector2 get(uint32_t index) const;

	void set(uint32_t index, const BoxedVector2* pt);

	const Winding2& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Winding2 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Winding2, false >
{
	static std::wstring typeName() {
		return L"traktor.Winding2";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedWinding2 >(value.getObjectUnsafe());
	}
    static Any set(const Winding2& value) {
        return Any::fromObject(new BoxedWinding2(value));
    }
    static const Winding2& get(const Any& value) {
		return static_cast< BoxedWinding2* >(value.getObject())->unbox();
    }
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Winding2&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Winding2&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedWinding2 >(value.getObjectUnsafe());
	}
    static Any set(const Winding2& value) {
        return Any::fromObject(new BoxedWinding2(value));
    }
    static const Winding2& get(const Any& value) {
		return static_cast< BoxedWinding2* >(value.getObject())->unbox();
	}
};

}
