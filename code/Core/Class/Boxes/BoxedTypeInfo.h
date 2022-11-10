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
class T_DLLCLASS BoxedTypeInfo : public Boxed
{
	T_RTTI_CLASS;

public:
	BoxedTypeInfo();

	explicit BoxedTypeInfo(const TypeInfo& value);

	std::wstring getName() const;

	int32_t getSize() const;

	int32_t getVersion() const;

	Ref< ITypedObject > createInstance() const;

	static Ref< BoxedTypeInfo > find(const std::wstring& name);

	static Ref< class BoxedRefArray > findAllOf(const BoxedTypeInfo* typeInfo, bool inclusive);

	const TypeInfo& unbox() const { return m_value; }

	virtual std::wstring toString() const override final;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	const TypeInfo& m_value;
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny< TypeInfo, false >
{
	static std::wstring typeName() {
		return L"traktor.TypeInfo";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTypeInfo >(value.getObjectUnsafe());
	}
	static Any set(const TypeInfo& value) {
		return Any::fromObject(new BoxedTypeInfo(value));
	}
	static const TypeInfo& get(const Any& value) {
		return static_cast< BoxedTypeInfo* >(value.getObject())->unbox();
	}
};

/*!
 * \ingroup Core
 */
template < >
struct CastAny< const TypeInfo&, false >
{
	static std::wstring typeName() {
		return L"const TypeInfo&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< BoxedTypeInfo >(value.getObjectUnsafe());
	}
	static Any set(const TypeInfo& value) {
		return Any::fromObject(new BoxedTypeInfo(value));
	}
	static const TypeInfo& get(const Any& value) {
		return static_cast< BoxedTypeInfo* >(value.getObject())->unbox();
	}
};

}
