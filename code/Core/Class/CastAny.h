/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Any.h"
#include "Core/Misc/String.h"
#include "Core/Math/Scalar.h"
#include "Core/Meta/Traits.h"

namespace traktor
{

/*! \name Any cast templates */
/*! \ingroup Core */
/*! \{ */

template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct CastAny
{
	T_NO_COPY_CLASS(CastAny);
};

template < >
struct CastAny < Any, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"Any";
	}
	static bool accept(const Any& value) {
		return true;
	}
	static Any set(const Any& value) {
		return value;
	}
	static const Any& get(const Any& value) {
		return value;
	}
};

template < >
struct CastAny < const Any&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"const Any&";
	}
	static bool accept(const Any& value) {
		return true;
	}
	static Any set(const Any& value) {
		return value;
	}
	static const Any& get(const Any& value) {
		return value;
	}
};

template < >
struct CastAny < bool, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"bool";
	}
	static bool accept(const Any& value) {
		return value.isBoolean();
	}
	static Any set(bool value) {
		return Any::fromBoolean(value);
	}
	static bool get(const Any& value) {
		return value.getBoolean();
	}
};

template < >
struct CastAny < int8_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"int8_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int8_t value) {
		return Any::fromInt32(value);
	}
	static int8_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint8_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"uint8_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint8_t value) {
		return Any::fromInt32(uint8_t(value));
	}
	static uint8_t get(const Any& value) {
		return (uint8_t)value.getInt32();
	}
};

template < >
struct CastAny < int16_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"int16_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int16_t value) {
		return Any::fromInt32(value);
	}
	static int16_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint16_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"uint16_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint16_t value) {
		return Any::fromInt32(uint16_t(value));
	}
	static uint16_t get(const Any& value) {
		return (uint16_t)value.getInt32();
	}
};

template < >
struct CastAny < int32_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"int32_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int32_t value) {
		return Any::fromInt32(value);
	}
	static int32_t get(const Any& value) {
		return value.getInt32();
	}
};

template < >
struct CastAny < uint32_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"uint32_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint32_t value) {
		return Any::fromInt32(int32_t(value));
	}
	static uint32_t get(const Any& value) {
		return (uint32_t)value.getInt32();
	}
};

template < >
struct CastAny < int64_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"int64_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(int64_t value) {
		return Any::fromInt64(value);
	}
	static int64_t get(const Any& value) {
		return value.getInt64();
	}
};

template < >
struct CastAny < uint64_t, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"uint64_t";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(uint64_t value) {
		return Any::fromInt64(int64_t(value));
	}
	static uint64_t get(const Any& value) {
		return (uint64_t)value.getInt64();
	}
};

template < >
struct CastAny < float, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"float";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(float value) {
		return Any::fromFloat(value);
	}
	static float get(const Any& value) {
		return value.getFloat();
	}
};

template < >
struct CastAny < double, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"double";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(double value) {
		return Any::fromFloat(float(value));
	}
	static double get(const Any& value) {
		return double(value.getFloat());
	}
};

template < >
struct CastAny < Scalar, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"Scalar";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < const Scalar&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"const Scalar&";
	}
	static bool accept(const Any& value) {
		return value.isNumeric();
	}
	static Any set(const Scalar& value) {
		return Any::fromFloat(float(value));
	}
	static Scalar get(const Any& value) {
		return Scalar(value.getFloat());
	}
};

template < >
struct CastAny < std::string, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"std::string";
	}
	static bool accept(const Any& value) {
		return value.isString() || value.isNumeric();
	}
	static Any set(const std::string& value) {
		return Any::fromString(value);
	}
	static std::string get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const std::string&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"const std::string&";
	}
	static bool accept(const Any& value) {
		return value.isString() || value.isNumeric();
	}
	static Any set(const std::string& value) {
		return Any::fromString(value);
	}
	static std::string get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < std::wstring, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"std::wstring";
	}
	static bool accept(const Any& value) {
		return value.isString() || value.isNumeric();
	}
	static Any set(const std::wstring& value) {
		return Any::fromString(value);
	}
	static std::wstring get(const Any& value) {
		return value.getWideString();
	}
};

template < >
struct CastAny < const std::wstring&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return L"const std::wstring&";
	}
	static bool accept(const Any& value) {
		return value.isString() || value.isNumeric();
	}
	static Any set(const std::wstring& value) {
		return Any::fromString(value);
	}
	static std::wstring get(const Any& value) {
		return value.getWideString();
	}
};

template < typename Type >
struct CastAny < Ref< Type >, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return str(L"Ref< %ls >", type_name< Type >());
	}
	static bool accept(const Any& value) {
		return value.isVoid() || value.isObject< Type >();
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		if (!value.isVoid())
			return static_cast< Type* >(value.getObjectUnsafe());
		else
			return nullptr;
	}
};

template < typename Type >
struct CastAny < const Ref< Type >&, false >
{
	T_NO_COPY_CLASS(CastAny);
	static std::wstring typeName() {
		return str(L"const Ref< %ls >&", type_name< Type >());
	}
	static bool accept(const Any& value) {
		return value.isVoid() || value.isObject< Type >();
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		if (!value.isVoid())
			return static_cast< Type* >(value.getObjectUnsafe());
		else
			return nullptr;
	}
};

template < >
struct CastAny < ITypedObject*, true >
{
	T_NO_COPY_CLASS(CastAny);

	typedef typename IsConst< ITypedObject* >::type_t no_const_type_t;

	static std::wstring typeName() {
		return type_name< ITypedObject >();
	}
	static bool accept(const Any& value) {
		return value.isVoid() || value.isObject();
	}
	static Any set(ITypedObject* value) {
		return Any::fromObject(value);
	}
	static ITypedObject* get(const Any& value) {
		if (!value.isVoid())
			return value.getObjectUnsafe();
		else
			return nullptr;
	}
};

template < typename Type >
struct CastAny < Type, false >
{
	T_NO_COPY_CLASS(CastAny);

	typedef typename IsConst< typename IsReference< Type >::base_t >::type_t type_t;

	static std::wstring typeName() {
		return type_name< Type >();
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const type_t& value) {
		return Any::fromObject(new type_t(value));
	}
	static Type get(const Any& value) {
		return Type(*static_cast< type_t* >(value.getObjectUnsafe()));
	}
};

template < typename Type >
struct CastAny < Type, true >
{
	T_NO_COPY_CLASS(CastAny);

	typedef typename IsConst< Type >::type_t no_const_type_t;

	static std::wstring typeName() {
		return type_name< Type >();
	}
	static bool accept(const Any& value) {
		return value.isVoid() || value.isObject< Type >();
	}
	static Any set(Type value) {
		return Any::fromObject((ITypedObject*)(value));
	}
	static Type get(const Any& value) {
		if (!value.isVoid())
			return static_cast< Type >(value.getObjectUnsafe());
		else
			return nullptr;
	}
};

/*! \} */

}
