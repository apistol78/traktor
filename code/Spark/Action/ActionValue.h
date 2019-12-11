#pragma once

#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/TString.h"
#include "Spark/Action/ActionTypes.h"
#include "Spark/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace spark
	{

class ActionContext;
class ActionObject;

/*! Action value.
 * \ingroup Spark
 *
 * An action value can be of several different
 * types and are dynamically cast as needed.
 */
class T_DLLCLASS ActionValue : public Collectable::IWeakRefDispose
{
public:
	enum Type
	{
		AvtUndefined,
		AvtBoolean,
		AvtInteger,
		AvtFloat,
		AvtString,
		AvtObject,
		AvtObjectWeak
	};

	ActionValue();

	ActionValue(const ActionValue& v);

#if defined(T_CXX11)
	ActionValue(ActionValue&& v);
#endif

	explicit ActionValue(bool b);

	explicit ActionValue(int32_t i);

	explicit ActionValue(float f);

	explicit ActionValue(const char* s, int32_t id = -1);

	explicit ActionValue(const std::string& s, int32_t id = -1);

	explicit ActionValue(const wchar_t* s, int32_t id = -1);

	explicit ActionValue(const std::wstring& s, int32_t id = -1);

	explicit ActionValue(ActionObject* o);

	explicit ActionValue(ActionObject* o, bool weak);

	virtual ~ActionValue();

	/*! Clear value. */
	void clear();

	/*! Cast to boolean. */
	ActionValue toBoolean() const { return ActionValue(getBoolean()); }

	/*! Cast to integer. */
	ActionValue toInteger() const { return ActionValue(getInteger()); }

	/*! Cast to float. */
	ActionValue toFloat() const { return ActionValue(getFloat()); }

	/*! Cast to string. */
	ActionValue toString() const { return ActionValue(getString()); }

	/*! Get type of value. */
	Type getType() const { return m_type; }

	/*! Check if undefined. */
	bool isUndefined() const { return m_type == AvtUndefined; }

	/*! Check if boolean. */
	bool isBoolean() const { return m_type == AvtBoolean; }

	/*! Check if integer. */
	bool isInteger() const { return m_type == AvtInteger; }

	/*! Check if float. */
	bool isFloat() const { return m_type == AvtFloat; }

	/*! Check if number. */
	bool isNumeric() const { return m_type == AvtInteger || m_type == AvtFloat; }

	/*! Check if string. */
	bool isString() const { return m_type == AvtString; }

	/*! Check if strong object. */
	bool isObjectStrong() const { return m_type == AvtObject; }

	/*! Check if weak object. */
	bool isObjectWeak() const { return m_type == AvtObjectWeak; }

	/*! Check if object. */
	bool isObject() const { return m_type == AvtObject || m_type == AvtObjectWeak; }

	/*! Check if object. */
	template < typename ObjectType >
	bool isObject() const { return (m_type == AvtObject || m_type == AvtObjectWeak) && is_a< ObjectType >(m_value.o); }

	/*! Get boolean value. */
	bool getBoolean() const;

	/*! Get integer value. */
	int32_t getInteger() const;

	/*! Get float value. */
	float getFloat() const;

	/*! Get string value. */
	std::string getString() const;

	/*! Get wide string value. */
	std::wstring getWideString() const;

	/*! Get object. */
	ActionObject* getObject() const { return (m_type == AvtObject || m_type == AvtObjectWeak) ? m_value.o : 0; }

	/*! Get object. */
	template < typename ObjectType >
	ObjectType* getObject() const { return dynamic_type_cast< ObjectType* >(getObject()); }

	/*! Get object always, ie. create boxes if not a object. */
	Ref< ActionObject > getObjectAlways(ActionContext* context) const;

	/*! Get object always, ie. create boxes if not a object. */
	template < typename ObjectType >
	Ref< ObjectType > getObjectAlways(ActionContext* context) const { return dynamic_type_cast< ObjectType* >(getObjectAlways(context)); }

	/*! Get string hidden identifier. */
	int32_t getStringId() const;

	/*! Serialize value. */
	void serialize(ISerializer& s);

	/*! Copy value. */
	ActionValue& operator = (const ActionValue& v);

#if defined(T_CXX11)
	/*! Move value. */
	ActionValue& operator = (ActionValue&& v);
#endif

	/*! Add */
	ActionValue operator + (const ActionValue& r) const;

	/*! Subtract */
	ActionValue operator - (const ActionValue& r) const;

	/*! Multiply */
	ActionValue operator * (const ActionValue& r) const;

	/*! Divide */
	ActionValue operator / (const ActionValue& r) const;

	/*! Compare equal. */
	bool operator == (const ActionValue& r) const;

private:
	union Value
	{
		bool b;
		int32_t i;
		float f;
		char* s;
		ActionObject* o;
	};

	Type m_type;
	Value m_value;

	virtual void disposeReference(Collectable* collectable);
};

	}
}

