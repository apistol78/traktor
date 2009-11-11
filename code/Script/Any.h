#ifndef traktor_script_Any_H
#define traktor_script_Any_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Any value container.
 * \ingroup Script
 *
 * Container class which will act
 * as a value placeholder when communicating
 * back and forth to script land.
 */
class T_DLLCLASS Any : public Object
{
	T_RTTI_CLASS(Any)

public:
	Any();

	Any(const Any& src);

	explicit Any(bool value);

	explicit Any(int32_t value);

	explicit Any(float value);

	explicit Any(const std::wstring& value);

	explicit Any(Object* value);

	virtual ~Any();

	bool getBoolean() const;

	int32_t getInteger() const;

	float getFloat() const;

	std::wstring getString() const;

	Ref< Object > getObject() const;

	bool isVoid() const { return m_type == AtVoid; }

	bool isBoolean() const { return m_type == AtBoolean; }

	bool isInteger() const { return m_type == AtInteger; }

	bool isFloat() const { return m_type == AtFloat; }

	bool isString() const { return m_type == AtString; }

	bool isObject() const { return m_type == AtObject; }

	Any& operator = (const Any& src);

private:
	typedef Ref< Object > ref_object_t;

	enum AnyType
	{
		AtVoid,
		AtBoolean,
		AtInteger,
		AtFloat,
		AtString,
		AtObject
	};

	union AnyData
	{
		bool m_boolean;
		int32_t m_integer;
		float m_float;
		wchar_t* m_string;
		ref_object_t* m_object;
	};

	AnyType m_type;
	AnyData m_data;
};

	}
}

#endif	// traktor_script_Any_H
