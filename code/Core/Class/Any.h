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
	enum AnyType
	{
		AtVoid,
		AtBoolean,
		AtInt32,
		AtInt64,
		AtFloat,
		AtString,
		AtObject
	};

	Any();

	Any(const Any& src);

#if defined(T_CXX11)
	Any(Any&& src);
#endif

	virtual ~Any();

	/*! Create Any from boolean. */
	static Any fromBoolean(bool value);

	/*! Create Any from 32-bit integer. */
	static Any fromInt32(int32_t value);

	/*! Create Any from 64-bit integer. */
	static Any fromInt64(int64_t value);

	/*! Create Any from 32-bit float. */
	static Any fromFloat(float value);

	/*! Create Any from UTF-8 string. */
	static Any fromString(const char* value);

	/*! Create Any from UTF-8 string. */
	static Any fromString(const std::string& value);

	/*! Create Any from wide string. */
	static Any fromString(const wchar_t* value);

	/*! Create Any from wide string. */
	static Any fromString(const std::wstring& value);

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

	/*! Get UTF-8 string from Any, cast if necessary. */
	std::string getString() const;

	/*! Get wide string from Any, cast if necessary. */
	std::wstring getWideString() const;

	/*! Get object from Any, cast if necessary. */
	ITypedObject* getObject() const { return m_type == AtObject ? m_data.m_object : nullptr; }

	/*! Get object from Any, cast if necessary. */
	template < typename ObjectType >
	ObjectType* getObject() const { return m_type == AtObject ? dynamic_type_cast< ObjectType* >(m_data.m_object) : nullptr; }

	/*! Get type of contained value. */
	AnyType getType() const { return m_type; }

	/*! Check if contained value is void. */
	bool isVoid() const { return m_type == AtVoid; }

	/*! Check if contained value is boolean. */
	bool isBoolean() const { return m_type == AtBoolean; }

	/*! Check if contained value is 32-bit integer. */
	bool isInt32() const { return m_type == AtInt32; }

	/*! Check if contained value is 64-bit integer. */
	bool isInt64() const { return m_type == AtInt64; }

	/*! Check if contained value is 32-bit float. */
	bool isFloat() const { return m_type == AtFloat; }

	/*! Check if contained value is string. */
	bool isString() const { return m_type == AtString; }

	/*! Check if contained value is object. */
	bool isObject() const { return m_type == AtObject; }

	/*! Check if contained value is object. */
	template < typename ObjectType >
	bool isObject() const { return m_type == AtObject && is_a< ObjectType >(m_data.m_object); }

	/*! Check if contained value is numeric. */
	bool isNumeric() const { return isInt32() || isInt64() || isFloat(); }

	// \name Unsafe accessors.
	// \{

	/*! Get boolean from Any, assuming correct type. */
	bool getBooleanUnsafe() const { T_ASSERT(m_type == AtBoolean); return m_data.m_boolean; }

	/*! Get 32-bit integer from Any, assuming correct type. */
	int32_t getInt32Unsafe( ) const { T_ASSERT(m_type == AtInt32); return m_data.m_int32; }

	/*! Get 64-bit integer from Any, assuming correct type. */
	int64_t getInt64Unsafe( ) const { T_ASSERT(m_type == AtInt64); return m_data.m_int64; }

	/*! Get 32-bit float from Any, assuming correct type. */
	float getFloatUnsafe() const { T_ASSERT(m_type == AtFloat); return m_data.m_float; }

	/*! Get string from Any, assuming correct type. */
	std::string getStringUnsafe() const { T_ASSERT(m_type == AtString); return m_data.m_string; }

	/*! Get object from Any, assuming correct type. */
	ITypedObject* getObjectUnsafe() const { T_ASSERT(m_type == AtObject); return m_data.m_object; }

	/*! Get object from Any, assuming correct type. */
	template < typename ObjectType >
	ObjectType* getObjectUnsafe() const { T_ASSERT(m_type == AtObject); return mandatory_non_null_type_cast< ObjectType* >(m_data.m_object); }

	// \}

	Any& operator = (const Any& src);

#if defined(T_CXX11)
	Any& operator = (Any&& src);
#endif

	std::wstring format() const;

private:
	union AnyData
	{
		bool m_boolean;
		int32_t m_int32;
		int64_t m_int64;
		float m_float;
		char* m_string;
		ITypedObject* m_object;
	};

	AnyType m_type;
	AnyData m_data;
};

}
