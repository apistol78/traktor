/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
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
class T_DLLCLASS BoxedGuid : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedGuid() = default;

	explicit BoxedGuid(const Guid& value);

	explicit BoxedGuid(const std::wstring& value);

	static Guid create() { return Guid::create(); }

	void set(const std::wstring& str) { m_value = Guid(str); }

	std::wstring format() const { return m_value.format(); }

	bool isValid() const { return m_value.isValid(); }

	bool isNull() const { return m_value.isNull(); }

	bool isNotNull() const { return m_value.isNotNull(); }

	const Guid& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Guid m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < Guid, false >
{
	static std::wstring typeName() {
		return L"traktor.Guid";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}
	static const Guid& get(const Any& value) {
		return static_cast< BoxedGuid* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const Guid&, false >
{
	static std::wstring typeName() {
		return L"const traktor.Guid&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedGuid >(value.getObjectUnsafe());
	}
	static Any set(const Guid& value) {
		return Any::fromObject(new BoxedGuid(value));
	}
	static const Guid& get(const Any& value) {
		return static_cast< BoxedGuid* >(value.getObject())->unbox();
	}
};

}
