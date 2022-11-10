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
#include "Core/Math/Winding3.h"

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
class T_DLLCLASS BoxedWinding3 : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedWinding3() = default;

	explicit BoxedWinding3(const Winding3& value);

	void clear();

	void push(const BoxedVector4* pt);

	void reserve(uint32_t size);

	void resize(uint32_t size);

	uint32_t size() const;

	bool empty() const;

	Vector4 get(uint32_t index) const;

	void set(uint32_t index, const BoxedVector4* pt);

	const Winding3& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Winding3 m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Winding3, false >
{
	static std::wstring typeName() {
		return L"traktor.Winding3";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedWinding3 >(value.getObjectUnsafe());
	}
    static Any set(const Winding3& value) {
        return Any::fromObject(new BoxedWinding3(value));
    }
    static const Winding3& get(const Any& value) {
		return static_cast< BoxedWinding3* >(value.getObject())->unbox();
    }
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Winding3&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Winding3&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedWinding3 >(value.getObjectUnsafe());
	}
    static Any set(const Winding3& value) {
        return Any::fromObject(new BoxedWinding3(value));
    }
    static const Winding3& get(const Any& value) {
		return static_cast< BoxedWinding3* >(value.getObject())->unbox();
	}
};

}
