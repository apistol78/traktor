/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Core/Ref.h"
#include "Core/Rtti/ITypedObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ITypedObject;

/*! Any value container.
 * \ingroup Core
 *
 * Container class which will act
 * as a value placeholder when communicating
 * back and forth to script land.
 */
class T_DLLCLASS Any
{
public:
	enum class Type : uint8_t
	{
		Void,
		Boolean,
		Int32,
		Int64,
		Float,
		Double,
		String,
		Object
	};

	Any()
	:	m_type(Type::Void)
	{
	}

	Any(const Any& src);

	Any(Any&& src) noexcept
	:	m_type(src.m_type)
	,	m_data(src.m_data)
	{
		src.m_type = Type::Void;
	}

	virtual ~Any();

	/*! Create Any from boolean. */
	static Any fromBoolean(bool value);

	/*! Create Any from 32-bit integer. */
	static Any fromInt32(int32_t value);

	/*! Create Any from 64-bit integer. */
	static Any fromInt64(int64_t value);

	/*! Create Any from 32-bit float. */
	static Any fromFloat(float value);

	/*! Create Any from 64-bit float. */
	static Any fromDouble(double value);

	/*! Create Any from UTF-8 string. */
	static Any fromString(const char* value);

	/*! Create Any from UTF-8 string. */
	static Any fromString(const std::string_view& value);

	/*! Create Any from wide string. */
	static Any fromString(const wchar_t* value);

	/*! Create Any from wide string. */
	static Any fromString(const std::wstring_view& value);

	/*! Create Any from object. */
	static Any fromObject(ITypedObject* value);

	/*! Get boolean from Any, cast if necessary. */
	bool getBoolean() const;

	/*! Get 32-bit integer from Any, cast if necessary. */
	int32_t getInt32() const;

	/*! Get 64-bit integer from Any, cast if necessary. */
	int64_t getInt64() const;

	/*! Get 32-bit float from Any, cast if necessary. */
	float getFloat() const;

	/*! Get 64-bit float from Any, cast if necessary. */
	double getDouble() const;

	/*! Get UTF-8 string from Any, cast if necessary. */
	std::string getString() const;

	/*! Get wide string from Any, cast if necessary. */
	std::wstring getWideString() const;

	/*! Get object from Any, cast if necessary. */
	inline ITypedObject* getObject() const { return m_type == Type::Object ? m_data.m_object : nullptr; }

	/*! Get object from Any, cast if necessary. */
	template < typename ObjectType >
	inline ObjectType* getObject() const { return m_type == Type::Object ? dynamic_type_cast< ObjectType* >(m_data.m_object) : nullptr; }

	/*! Get type of contained value. */
	inline Type getType() const { return m_type; }

	/*! Check if contained value is void. */
	inline bool isVoid() const { return m_type == Type::Void; }

	/*! Check if contained value is boolean. */
	inline bool isBoolean() const { return m_type == Type::Boolean; }

	/*! Check if contained value is 32-bit integer. */
	inline bool isInt32() const { return m_type == Type::Int32; }

	/*! Check if contained value is 64-bit integer. */
	inline bool isInt64() const { return m_type == Type::Int64; }

	/*! Check if contained value is 32-bit float. */
	inline bool isFloat() const { return m_type == Type::Float; }

	/*! Check if contained value is 64-bit float. */
	inline bool isDouble() const { return m_type == Type::Double; }

	/*! Check if contained value is string. */
	inline bool isString() const { return m_type == Type::String; }

	/*! Check if contained value is object. */
	inline bool isObject() const { return m_type == Type::Object; }

	/*! Check if contained value is object. */
	template < typename ObjectType >
	inline bool isObject() const { return m_type == Type::Object && is_a< ObjectType >(m_data.m_object); }

	/*! Check if contained value is numeric. */
	inline bool isNumeric() const { return isInt32() || isInt64() || isFloat() || isDouble(); }

	// \name Unsafe accessors.
	// \{

	/*! Get boolean from Any, assuming correct type. */
	inline bool getBooleanUnsafe() const { T_ASSERT(m_type == Type::Boolean); return m_data.m_boolean; }

	/*! Get 32-bit integer from Any, assuming correct type. */
	inline int32_t getInt32Unsafe( ) const { T_ASSERT(m_type == Type::Int32); return m_data.m_int32; }

	/*! Get 64-bit integer from Any, assuming correct type. */
	inline int64_t getInt64Unsafe( ) const { T_ASSERT(m_type == Type::Int64); return m_data.m_int64; }

	/*! Get 32-bit float from Any, assuming correct type. */
	inline float getFloatUnsafe() const { T_ASSERT(m_type == Type::Float); return m_data.m_float; }

	/*! Get 64-bit double from Any, assuming correct type. */
	inline double getDoubleUnsafe() const { T_ASSERT(m_type == Type::Double); return m_data.m_double; }

	/*! Get string from Any, assuming correct type. */
	inline std::string getStringUnsafe() const { T_ASSERT(m_type == Type::String); return m_data.m_string; }

	/*! Get string from Any, assuming correct type. */
	inline const char* getCStringUnsafe() const { T_ASSERT(m_type == Type::String); return m_data.m_string; }

	/*! Get object from Any, assuming correct type. */
	inline ITypedObject* getObjectUnsafe() const { T_ASSERT(m_type == Type::Object); return m_data.m_object; }

	/*! Get object from Any, assuming correct type. */
	template < typename ObjectType >
	inline ObjectType* getObjectUnsafe() const { T_ASSERT(m_type == Type::Object); return mandatory_non_null_type_cast< ObjectType* >(m_data.m_object); }

	// \}

	Any& operator = (const Any& src);

	Any& operator = (Any&& src) noexcept;

	std::wstring format() const;

private:
	union Data
	{
		bool m_boolean;
		int32_t m_int32;
		int64_t m_int64;
		float m_float;
		double m_double;
		char* m_string;
		ITypedObject* m_object;
	};

	Type m_type;
	Data m_data;

	explicit Any(Type type)
	:	m_type(type)
	{
	}
};

}
