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
#include "Core/Math/Color4ub.h"

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
class T_DLLCLASS BoxedColor4ub : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedColor4ub() = default;

	explicit BoxedColor4ub(const Color4ub& value);

	explicit BoxedColor4ub(uint8_t r, uint8_t g, uint8_t b);

	explicit BoxedColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	uint8_t getRed() const { return m_value.r; }

	uint8_t getGreen() const { return m_value.g; }

	uint8_t getBlue() const { return m_value.b; }

	uint8_t getAlpha() const { return m_value.a; }

	uint32_t getARGB() const { return m_value.getARGB(); }

	void setRed(uint8_t red) { m_value.r = red; }

	void setGreen(uint8_t green) { m_value.g = green; }

	void setBlue(uint8_t blue) { m_value.b = blue; }

	void setAlpha(uint8_t alpha) { m_value.a = alpha; }

	const Color4ub& unbox() const { return m_value; }

	std::wstring formatRGB() const;

	std::wstring formatARGB() const;

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Color4ub m_value = Color4ub(255, 255, 255, 255);
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Color4ub, false >
{
	static std::wstring typeName() {
		return L"traktor.Color4ub";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static const Color4ub& get(const Any& value) {
		return static_cast< BoxedColor4ub* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Color4ub&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Color4ub&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedColor4ub >(value.getObjectUnsafe());
	}
	static Any set(const Color4ub& value) {
		return Any::fromObject(new BoxedColor4ub(value));
	}
	static const Color4ub& get(const Any& value) {
		return static_cast< BoxedColor4ub* >(value.getObject())->unbox();
	}
};

}
