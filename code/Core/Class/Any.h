/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Any_H
#define traktor_Any_H

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

/*! \brief Any value container.
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

	static Any fromBoolean(bool value);

	static Any fromInt32(int32_t value);

	static Any fromInt64(int64_t value);

	static Any fromFloat(float value);

	static Any fromString(const char* value);

	static Any fromString(const std::string& value);

	static Any fromString(const wchar_t* value);

	static Any fromString(const std::wstring& value);

	static Any fromObject(ITypedObject* value);

	bool getBoolean() const;

	int32_t getInt32() const;

	int64_t getInt64() const;

	float getFloat() const;

	std::string getString() const;

	std::wstring getWideString() const;

	ITypedObject* getObject() const { return m_type == AtObject ? m_data.m_object : 0; }

	AnyType getType() const { return m_type; }

	bool isVoid() const { return m_type == AtVoid; }

	bool isBoolean() const { return m_type == AtBoolean; }

	bool isInt32() const { return m_type == AtInt32; }

	bool isInt64() const { return m_type == AtInt64; }

	bool isFloat() const { return m_type == AtFloat; }

	bool isString() const { return m_type == AtString; }

	bool isObject() const { return m_type == AtObject; }

	template< typename ObjectType >
	bool isObject() const { return m_type == AtObject && is_a< ObjectType >(m_data.m_object); }

	bool isNumeric() const { return isInt32() || isInt64() || isFloat(); }

	// \name Unsafe accessors.
	// \{

	bool getBooleanUnsafe() const { T_ASSERT (m_type == AtBoolean); return m_data.m_boolean; }

	int32_t getInt32Unsafe( ) const { T_ASSERT (m_type == AtInt32); return m_data.m_int32; }

	int64_t getInt64Unsafe( ) const { T_ASSERT (m_type == AtInt64); return m_data.m_int64; }

	float getFloatUnsafe() const { T_ASSERT (m_type == AtFloat); return m_data.m_float; }

	std::string getStringUnsafe() const { T_ASSERT (m_type == AtString); return m_data.m_string; }

	ITypedObject* getObjectUnsafe() const { T_ASSERT (m_type == AtObject); return m_data.m_object; }

	template < typename ObjectType >
	ObjectType* getObjectUnsafe() const { T_ASSERT (m_type == AtObject); return mandatory_non_null_type_cast< ObjectType* >(m_data.m_object); }

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

#endif	// traktor_Any_H
