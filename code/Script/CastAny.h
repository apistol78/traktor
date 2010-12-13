#ifndef traktor_script_CastAny_H
#define traktor_script_CastAny_H

#include "Core/Io/Utf8Encoding.h"
#include "Core/Meta/Traits.h"
#include "Core/Misc/TString.h"
#include "Script/Any.h"
#include "Script/Boxes.h"

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
struct CastAny < Vector4, false >
{
	static Any set(const Vector4& value) {
		return Any(new BoxedVector4(value));
	}	
	static Vector4 get(const Any& value) {
		return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Vector4&, false >
{
	static Any set(const Vector4& value) {
		return Any(new BoxedVector4(value));
	}	
	static Vector4 get(const Any& value) {
		return checked_type_cast< BoxedVector4*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Quaternion, false >
{
	static Any set(const Quaternion& value) {
		return Any(new BoxedQuaternion(value));
	}
	static Quaternion get(const Any& value) {
		return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Quaternion&, false >
{
	static Any set(const Quaternion& value) {
		return Any(new BoxedQuaternion(value));
	}
	static Quaternion get(const Any& value) {
		return checked_type_cast< BoxedQuaternion*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < Transform, false >
{
	static Any set(const Transform& value) {
		return Any(new BoxedTransform(value));
	}
	static Transform get(const Any& value) {
		return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const Transform&, false >
{
	static Any set(const Transform& value) {
		return Any(new BoxedTransform(value));
	}
	static Transform get(const Any& value) {
		return checked_type_cast< BoxedTransform*, false >(value.getObject())->unbox();
	}
};

template < typename InnerType >
struct CastAny < RefArray< InnerType >, false >
{
	static Any set(const RefArray< InnerType >& value) {
		return Any(new BoxedArray(value));
	}
	static RefArray< InnerType > get(const Any& value) {
		return checked_type_cast< BoxedArray*, false >(value.getObject())->unbox();
	}
};

template < typename InnerType >
struct CastAny < const RefArray< InnerType >&, false >
{
	static Any set(const RefArray< InnerType >& value) {
		return Any(new BoxedArray(value));
	}
	static RefArray< InnerType > get(const Any& value) {
		return checked_type_cast< BoxedArray*, false >(value.getObject())->unbox();
	}
};

/*! \} */

	}
}

#endif	// traktor_script_CastAny_H
