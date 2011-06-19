#ifndef traktor_script_CastAny_H
#define traktor_script_CastAny_H

#include "Core/Io/Utf8Encoding.h"
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
	static Any set(Type value) {
		return Any(value);
	}
};

template < >
struct CastAny < Any, false >
{
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
	static Any set(bool value) {
		return Any(value);
	}
	static bool get(const Any& value) {
		return value.getBoolean();
	}
};

template < >
struct CastAny < int32_t, false >
{
	static Any set(int32_t value) {
		return Any(value);
	}
	static int32_t get(const Any& value) {
		return value.getInteger();
	}
};

template < >
struct CastAny < uint32_t, false >
{
	static Any set(uint32_t value) {
		return Any(int32_t(value));
	}
	static uint32_t get(const Any& value) {
		return (uint32_t)value.getInteger();
	}
};

template < >
struct CastAny < float, false >
{
	static Any set(float value) {
		return Any(value);
	}
	static float get(const Any& value) {
		return value.getFloat();
	}
};

template < >
struct CastAny < std::string, false >
{
	static Any set(const std::string& value) {
		return Any(mbstows(Utf8Encoding(), value));
	}
	static std::string get(const Any& value) {
		return wstombs(Utf8Encoding(), value.getString());
	}
};

template < >
struct CastAny < const std::string&, false >
{
	static Any set(const std::string& value) {
		return Any(mbstows(Utf8Encoding(), value));
	}
	static std::string get(const Any& value) {
		return wstombs(Utf8Encoding(), value.getString());
	}
};

template < >
struct CastAny < std::wstring, false >
{
	static Any set(const std::wstring& value) {
		return Any(value);
	}
	static std::wstring get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const std::wstring&, false >
{
	static Any set(const std::wstring& value) {
		return Any(value);
	}
	static std::wstring get(const Any& value) {
		return value.getString();
	}
};

template < >
struct CastAny < const wchar_t, true >
{
	static Any set(const wchar_t* value) {
		return Any(std::wstring(value));
	}
	static const wchar_t* get(const Any& value) {
		return value.getString().c_str();
	}
};

template < typename Type >
struct CastAny < Ref< Type >, false >
{
	static Any set(const Ref< Type >& value) {
		return Any(value);
	}
	static Ref< Type > get(const Any& value) {
		return checked_type_cast< Type*, false >(value.getObject());
	}
};

template < typename Type >
struct CastAny < const Ref< Type >&, false >
{
	static Any set(const Ref< Type >& value) {
		return Any(value);
	}
	static Ref< Type > get(const Any& value) {
		return checked_type_cast< Type*, false >(value.getObject());
	}
};

template < typename Type >
struct CastAny < Type, false >
{
	typedef typename IsReference< Type >::base_t type_t;

	static Any set(const type_t& value) {
		return Any(new type_t(value));
	}

	static Type get(const Any& value) {
		return Type(*checked_type_cast< type_t*, false >(value.getObject()));
	}
};

template < typename Type >
struct CastAny < Type, true >
{
	static Any set(Type value) {
		return Any(value);
	}
	static Type get(const Any& value) {
		return checked_type_cast< Type >(value.getObject());
	}
};

template < >
struct CastAny < const TypeInfo&, false >
{
	static Any set(const TypeInfo& value) {
		return Any(&value);
	}
	static const TypeInfo& get(const Any& value) {
		return *value.getType();
	}
};

template < >
struct CastAny < const TypeInfo, true >
{
	static Any set(const TypeInfo* value) {
		return Any(value);
	}
	static const TypeInfo* get(const Any& value) {
		return value.getType();
	}
};

/*! \} */

	}
}

#endif	// traktor_script_CastAny_H
