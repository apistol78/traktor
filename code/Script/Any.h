#ifndef traktor_script_Any_H
#define traktor_script_Any_H

#include "Core/Config.h"
#include "Core/Ref.h"
#include "Core/Rtti/TypeInfo.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ITypedObject;

	namespace script
	{

/*! \brief Any value container.
 * \ingroup Script
 *
 * Container class which will act
 * as a value placeholder when communicating
 * back and forth to script land.
 */
class T_DLLCLASS Any
{
public:
	Any();

	Any(const Any& src);

#if 0 // defined(T_CXX11)
	Any(Any&& src);
#endif

	virtual ~Any();

	static Any fromBoolean(bool value);

	static Any fromInteger(int32_t value);

	static Any fromFloat(float value);

	static Any fromString(const char* value);

	static Any fromString(const std::string& value);

	static Any fromString(const wchar_t* value);

	static Any fromString(const std::wstring& value);

	static Any fromObject(ITypedObject* value);

	static Any fromTypeInfo(const TypeInfo* value);

	bool getBoolean() const;

	int32_t getInteger() const;

	float getFloat() const;

	std::string getString() const;

	std::wstring getWideString() const;

	ITypedObject* getObject() const { return m_type == AtObject ? m_data.m_object : 0; }

	const TypeInfo* getTypeInfo() const { return m_type == AtTypeInfo ? m_data.m_typeInfo : 0; }

	bool isVoid() const { return m_type == AtVoid; }

	bool isBoolean() const { return m_type == AtBoolean; }

	bool isInteger() const { return m_type == AtInteger; }

	bool isFloat() const { return m_type == AtFloat; }

	bool isString() const { return m_type == AtString; }

	bool isObject() const { return m_type == AtObject; }

	bool isTypeInfo() const { return m_type == AtTypeInfo; }

	// \name Unsafe accessors.
	// \{

	bool getBooleanUnsafe() const { T_ASSERT (m_type == AtBoolean); return m_data.m_boolean; }

	int32_t getIntegerUnsafe( ) const { T_ASSERT (m_type == AtInteger); return m_data.m_integer; }

	float getFloatUnsafe() const { T_ASSERT (m_type == AtFloat); return m_data.m_float; }

	std::string getStringUnsafe() const { T_ASSERT (m_type == AtString); return m_data.m_string; }

	ITypedObject* getObjectUnsafe() const { T_ASSERT (m_type == AtObject); return m_data.m_object; }

	const TypeInfo* getTypeInfoUnsafe() const { T_ASSERT (m_type == AtTypeInfo); return m_data.m_typeInfo; }

	// \}

	Any& operator = (const Any& src);

#if defined(T_CXX11)
	Any& operator = (Any&& src);
#endif

private:
	enum AnyType
	{
		AtVoid,
		AtBoolean,
		AtInteger,
		AtFloat,
		AtString,
		AtObject,
		AtTypeInfo
	};

	union AnyData
	{
		bool m_boolean;
		int32_t m_integer;
		float m_float;
		char* m_string;
		ITypedObject* m_object;
		const TypeInfo* m_typeInfo;
	};

	AnyType m_type;
	AnyData m_data;
};

	}
}

#endif	// traktor_script_Any_H
