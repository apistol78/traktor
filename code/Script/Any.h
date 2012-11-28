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

class Object;

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

	explicit Any(bool value);

	explicit Any(int32_t value);

	explicit Any(float value);

	explicit Any(const char* value);

	explicit Any(const std::string& value);

	explicit Any(const wchar_t* value);

	explicit Any(const std::wstring& value);

	explicit Any(Object* value);

	explicit Any(const TypeInfo* value);

	virtual ~Any();

	bool getBoolean() const;

	int32_t getInteger() const;

	float getFloat() const;

	std::string getString() const;

	std::wstring getWideString() const;

	Object* getObject() const;

	const TypeInfo* getType() const;

	bool isVoid() const { return m_type == AtVoid; }

	bool isBoolean() const { return m_type == AtBoolean; }

	bool isInteger() const { return m_type == AtInteger; }

	bool isFloat() const { return m_type == AtFloat; }

	bool isString() const { return m_type == AtString; }

	bool isObject() const { return m_type == AtObject; }

	bool isType() const { return m_type == AtType; }

	// \name Unsafe accessors.
	// \{

	bool getBooleanUnsafe() const { T_ASSERT (m_type == AtBoolean); return m_data.m_boolean; }

	int32_t getIntegerUnsafe( ) const { T_ASSERT (m_type == AtInteger); return m_data.m_integer; }

	float getFloatUnsafe() const { T_ASSERT (m_type == AtFloat); return m_data.m_float; }

	std::string getStringUnsafe() const { T_ASSERT (m_type == AtString); return m_data.m_string; }

	Object* getObjectUnsafe() const { T_ASSERT (m_type == AtObject); return m_data.m_object; }

	const TypeInfo* getTypeUnsafe() const { T_ASSERT (m_type == AtType); return m_data.m_type; }

	// \}

	Any& operator = (const Any& src);

private:
	enum AnyType
	{
		AtVoid,
		AtBoolean,
		AtInteger,
		AtFloat,
		AtString,
		AtObject,
		AtType
	};

	union AnyData
	{
		bool m_boolean;
		int32_t m_integer;
		float m_float;
		char* m_string;
		Object* m_object;
		const TypeInfo* m_type;
	};

	AnyType m_type;
	AnyData m_data;
};

	}
}

#endif	// traktor_script_Any_H
