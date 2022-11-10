/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Boxes/BoxedRandom.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Math/RandomGeometry.h"

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
class T_DLLCLASS BoxedRandomGeometry : public BoxedRandom
{
	T_RTTI_CLASS;

public:
	BoxedRandomGeometry() = default;

	explicit BoxedRandomGeometry(const RandomGeometry& value);

	explicit BoxedRandomGeometry(uint32_t seed);

	Vector4 nextUnit() { return m_value.nextUnit(); }

	Vector4 nextHemi(const BoxedVector4* direction) { return m_value.nextHemi(direction->unbox()); }

	const RandomGeometry& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	RandomGeometry m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < RandomGeometry, false >
{
	static std::wstring typeName() {
		return L"traktor.RandomGeometry";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static const RandomGeometry& get(const Any& value) {
		return static_cast< BoxedRandomGeometry* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const RandomGeometry&, false >
{
	static std::wstring typeName() {
		return L"const traktor.RandomGeometry&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedRandomGeometry >(value.getObjectUnsafe());
	}
	static Any set(const RandomGeometry& value) {
		return Any::fromObject(new BoxedRandomGeometry(value));
	}
	static const RandomGeometry& get(const Any& value) {
		return static_cast< BoxedRandomGeometry* >(value.getObject())->unbox();
	}
};

}
