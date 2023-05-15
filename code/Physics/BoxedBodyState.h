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
#include "Physics/BodyState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

/*!
 * \ingroup Physics
 */
class T_DLLCLASS BoxedBodyState : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedBodyState() = default;

	explicit BoxedBodyState(const BodyState& value);

	const BodyState& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	BodyState m_value;
};

}

namespace traktor
{

/*!
 * \ingroup Physics
 */
template < >
struct CastAny < physics::BodyState, false >
{
	static std::wstring typeName() {
		return L"traktor.physics.BodyState";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< physics::BoxedBodyState >(value.getObjectUnsafe());
	}
	static Any set(const physics::BodyState& value) {
		return Any::fromObject(new physics::BoxedBodyState(value));
	}
	static const physics::BodyState& get(const Any& value) {
		return static_cast<physics::BoxedBodyState* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Physics
 */
template < >
struct CastAny < const physics::BodyState&, false >
{
	static std::wstring typeName() {
		return L"const traktor.physics.BodyState&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< physics::BoxedBodyState >(value.getObjectUnsafe());
	}
	static Any set(const physics::BodyState& value) {
		return Any::fromObject(new physics::BoxedBodyState(value));
	}
	static const physics::BodyState& get(const Any& value) {
		return static_cast< physics::BoxedBodyState* >(value.getObject())->unbox();
	}
};

}
