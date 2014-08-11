#ifndef traktor_script_CastAny_H
#define traktor_script_CastAny_H

#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/Scalar.h"
#include "Core/Meta/Traits.h"
#include "Core/Misc/TString.h"
#include "Script/Any.h"

namespace traktor
{
	namespace script
	{

/*! \name Any cast templates */
/*! \ingroup Script */
/*! \{ */

template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct CastAny
{
};

template < >
struct CastAny < Any, false >
{
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
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(int8_t value) {
		return Any::fromInteger(value);
	}
	static int8_t get(const Any& value) {
		return value.getInteger();
	}
};

template < >
struct CastAny < uint8_t, false >
{
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(uint8_t value) {
		return Any::fromInteger(uint8_t(value));
	}
	static uint8_t get(const Any& value) {
		return (uint8_t)value.getInteger();
	}
};

template < >
struct CastAny < int16_t, false >
{
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(int16_t value) {
		return Any::fromInteger(value);
	}
	static int16_t get(const Any& value) {
		return value.getInteger();
	}
};

template < >
struct CastAny < uint16_t, false >
{
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(uint16_t value) {
		return Any::fromInteger(uint16_t(value));
	}
	static uint16_t get(const Any& value) {
		return (uint16_t)value.getInteger();
	}
};

template < >
struct CastAny < int32_t, false >
{
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(int32_t value) {
		return Any::fromInteger(value);
	}
	static int32_t get(const Any& value) {
		return value.getInteger();
	}
};

template < >
struct CastAny < uint32_t, false >
{
	static bool accept(const Any& value) {
		return value.isInteger();
	}
	static Any set(uint32_t value) {
		return Any::fromInteger(int32_t(value));
	}
	static uint32_t get(const Any& value) {
		return (uint32_t)value.getInteger();
	}
};

template < >
struct CastAny < float, false >
{
	static bool accept(const Any& value) {
		return value.isFloat();
	}
	static Any set(float value) {
		return Any::fromFloat(value);
	}
	static float get(const Any& value) {
		return value.getFloat();
	}
};

template < >
struct CastAny < Scalar, false >
{
	static bool accept(const Any& value) {
		return value.isFloat();
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
	static bool accept(const Any& value) {
		return value.isFloat();
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
	static bool accept(const Any& value) {
		return value.isString();
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
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::string& value) {
		return Any::fromString(value);
	}
	static std::string get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const char, true >
{
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const char* value) {
		return Any::fromString(value);
	}
	static const char* get(const Any& value) {
		return value.getString().c_str();
	}
};

template < >
struct CastAny < std::wstring, false >
{
	static bool accept(const Any& value) {
		return value.isString();
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
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const std::wstring& value) {
		return Any::fromString(value);
	}
	static std::wstring get(const Any& value) {
		return value.getWideString();
	}
};

template < >
struct CastAny < const wchar_t, true >
{
	static bool accept(const Any& value) {
		return value.isString();
	}
	static Any set(const wchar_t* value) {
		return Any::fromString(value);
	}
	static const wchar_t* get(const Any& value) {
		return value.getWideString().c_str();
	}
};

template < typename Type >
struct CastAny < Ref< Type >, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		return checked_type_cast< Type*, false >(value.getObject());
	}
};

template < typename Type >
struct CastAny < const Ref< Type >&, false >
{
	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}
	static Any set(const Ref< Type >& value) {
		return Any::fromObject(const_cast< typename IsConst< Type >::type_t* >(value.ptr()));
	}
	static Ref< Type > get(const Any& value) {
		return checked_type_cast< Type*, false >(value.getObject());
	}
};

template < typename Type >
struct CastAny < Type, false >
{
	typedef typename IsConst< typename IsReference< Type >::base_t >::type_t type_t;

	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}

	static Any set(const type_t& value) {
		return Any::fromObject(new type_t(value));
	}

	static Type get(const Any& value) {
		return Type(*checked_type_cast< type_t*, false >(value.getObject()));
	}
};

template < typename Type >
struct CastAny < Type, true >
{
	typedef typename IsConst< Type >::type_t no_const_type_t;

	static bool accept(const Any& value) {
		return value.isObject() && is_a< Type >(value.getObjectUnsafe());
	}

	static Any set(Type value) {
		return Any::fromObject((ITypedObject*)(value));
	}

	static Type get(const Any& value) {
		return checked_type_cast< Type >(value.getObject());
	}
};

template < >
struct CastAny < const TypeInfo&, false >
{
	static bool accept(const Any& value) {
		return value.isTypeInfo();
	}
	static Any set(const TypeInfo& value) {
		return Any::fromTypeInfo(&value);
	}
	static const TypeInfo& get(const Any& value) {
		return *value.getTypeInfo();
	}
};

template < >
struct CastAny < const TypeInfo, true >
{
	static bool accept(const Any& value) {
		return value.isTypeInfo();
	}
	static Any set(const TypeInfo* value) {
		return Any::fromTypeInfo(value);
	}
	static const TypeInfo* get(const Any& value) {
		return value.getTypeInfo();
	}
};

/*! \} */

	}
}

#endif	// traktor_script_CastAny_H
