#ifndef traktor_flash_ActionValue_H
#define traktor_flash_ActionValue_H

#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/TString.h"
#include "Flash/Action/ActionTypes.h"
#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace flash
	{

class ActionContext;
class ActionObject;

/*! \brief Action value.
 * \ingroup Flash
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

	/*! \brief Clear value. */
	void clear();

	/*! \brief Cast to boolean. */
	ActionValue toBoolean() const { return ActionValue(getBoolean()); }

	/*! \brief Cast to integer. */
	ActionValue toInteger() const { return ActionValue(getInteger()); }

	/*! \brief Cast to float. */
	ActionValue toFloat() const { return ActionValue(getFloat()); }

	/*! \brief Cast to string. */
	ActionValue toString() const { return ActionValue(getString()); }

	/*! \brief Get type of value. */
	Type getType() const { return m_type; }

	/*! \brief Check if undefined. */
	bool isUndefined() const { return m_type == AvtUndefined; }

	/*! \brief Check if boolean. */
	bool isBoolean() const { return m_type == AvtBoolean; }

	/*! \brief Check if integer. */
	bool isInteger() const { return m_type == AvtInteger; }

	/*! \brief Check if float. */
	bool isFloat() const { return m_type == AvtFloat; }

	/*! \brief Check if number. */
	bool isNumeric() const { return m_type == AvtInteger || m_type == AvtFloat; }

	/*! \brief Check if string. */
	bool isString() const { return m_type == AvtString; }

	/*! \brief Check if strong object. */
	bool isObjectStrong() const { return m_type == AvtObject; }

	/*! \brief Check if weak object. */
	bool isObjectWeak() const { return m_type == AvtObjectWeak; }

	/*! \brief Check if object. */
	bool isObject() const { return m_type == AvtObject || m_type == AvtObjectWeak; }

	/*! \brief Check if object. */
	template < typename ObjectType >
	bool isObject() const { return (m_type == AvtObject || m_type == AvtObjectWeak) && is_a< ObjectType >(m_value.o); }

	/*! \brief Get boolean value. */
	bool getBoolean() const;

	/*! \brief Get integer value. */
	int32_t getInteger() const;

	/*! \brief Get float value. */
	float getFloat() const;

	/*! \brief Get string value. */
	std::string getString() const;

	/*! \brief Get wide string value. */
	std::wstring getWideString() const;

	/*! \brief Get object. */
	ActionObject* getObject() const { return (m_type == AvtObject || m_type == AvtObjectWeak) ? m_value.o : 0; }

	/*! \brief Get object. */
	template < typename ObjectType >
	ObjectType* getObject() const { return dynamic_type_cast< ObjectType* >(getObject()); }

	/*! \brief Get object always, ie. create boxes if not a object. */
	Ref< ActionObject > getObjectAlways(ActionContext* context) const;

	/*! \brief Get object always, ie. create boxes if not a object. */
	template < typename ObjectType >
	Ref< ObjectType > getObjectAlways(ActionContext* context) const { return dynamic_type_cast< ObjectType* >(getObjectAlways(context)); }

	/*! \brief Get string hidden identifier. */
	int32_t getStringId() const;

	/*! \brief Serialize value. */
	void serialize(ISerializer& s);

	/*! \brief Copy value. */
	ActionValue& operator = (const ActionValue& v);

#if defined(T_CXX11)
	/*! \brief Move value. */
	ActionValue& operator = (ActionValue&& v);
#endif

	/*! \brief Add */
	ActionValue operator + (const ActionValue& r) const;

	/*! \brief Subtract */
	ActionValue operator - (const ActionValue& r) const;

	/*! \brief Multiply */
	ActionValue operator * (const ActionValue& r) const;

	/*! \brief Divide */
	ActionValue operator / (const ActionValue& r) const;

	/*! \brief Compare equal. */
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

#endif	// traktor_flash_ActionValue_H
