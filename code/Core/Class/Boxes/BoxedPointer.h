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
class T_DLLCLASS BoxedPointer : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedPointer() = default;

	explicit BoxedPointer(void* ptr)
	:	m_ptr(ptr)
	{
	}

	void* ptr() const { return m_ptr; }

	bool null() const { return m_ptr == nullptr; }

	virtual std::wstring toString() const override final;

	void* unbox() const
	{
		return m_ptr;
	}

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	void* m_ptr = nullptr;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < void*, true >
{
	static std::wstring typeName() {
		return L"void*";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPointer >(value.getObjectUnsafe());
	}
	static Any set(void* value) {
		return Any::fromObject(new BoxedPointer(value));
	}
	static void* get(const Any& value) {
		return static_cast< BoxedPointer* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny < const void*, true >
{
	static std::wstring typeName() {
		return L"const void*";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedPointer >(value.getObjectUnsafe());
	}
	static Any set(const void* value) {
		return Any::fromObject(new BoxedPointer((void*)value));
	}
	static const void* get(const Any& value) {
		return static_cast< BoxedPointer* >(value.getObject())->unbox();
	}
};

}
