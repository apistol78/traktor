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
	namespace flash
	{

class ActionObject;

/*! \brief Action value.
 * \ingroup Flash
 *
 * An action value can be of several different
 * types and are dynamically cast as needed.
 */
class T_DLLCLASS ActionValue
{
public:
	enum Type
	{
		AvtUndefined,
		AvtBoolean,
		AvtNumber,
		AvtString,
		AvtObject
	};

	ActionValue();

	ActionValue(const ActionValue& v);

	explicit ActionValue(bool b);

	explicit ActionValue(avm_number_t n);

	explicit ActionValue(const char* s);

	explicit ActionValue(const std::string& s);

	explicit ActionValue(const wchar_t* s);

	explicit ActionValue(const std::wstring& s);

	explicit ActionValue(ActionObject* o);

	virtual ~ActionValue();

	/*! \brief Cast to boolean. */
	ActionValue toBoolean() const;

	/*! \brief Cast to number. */
	ActionValue toNumber() const;

	/*! \brief Cast to string. */
	ActionValue toString() const;

	Type getType() const { return m_type; }

	/*! \brief Check if undefined. */
	bool isUndefined() const { return m_type == AvtUndefined; }

	/*! \brief Check if boolean. */
	bool isBoolean() const { return m_type == AvtBoolean; }

	/*! \brief Check if number. */
	bool isNumeric() const { return m_type == AvtNumber; }

	/*! \brief Check if string. */
	bool isString() const { return m_type == AvtString; }

	/*! \brief Check if object. */
	bool isObject() const { return m_type == AvtObject; }

	/*! \brief Get boolean value. */
	bool getBoolean() const { T_ASSERT_M (m_type == AvtBoolean, L"Incorrect type"); return m_value.b; }

	/*! \brief Get number value. */
	avm_number_t getNumber() const { T_ASSERT_M (m_type == AvtNumber, L"Incorrect type"); return m_value.n; }

	/*! \brief Get string value. */
	std::string getString() const { T_ASSERT_M (m_type == AvtString, L"Incorrect type"); return m_value.s; }

	/*! \brief Get wide string value. */
	std::wstring getWideString() const { T_ASSERT_M (m_type == AvtString, L"Incorrect type"); return mbstows(Utf8Encoding(), m_value.s); }

	/*! \brief Get object value. */
	ActionObject* getObject() const { T_ASSERT_M (m_type == AvtObject, L"Incorrect type"); return m_value.o; }

	/*! \brief Get boolean value safe. */
	bool getBooleanSafe() const;

	/*! \brief Get number value safe. */
	avm_number_t getNumberSafe() const;

	/*! \brief Get string value safe. */
	std::string getStringSafe() const;

	/*! \brief Get wide string value safe. */
	std::wstring getWideStringSafe() const;

	/*! \brief Get object safe. */
	Ref< ActionObject > getObjectSafe() const;

	ActionValue& operator = (const ActionValue& v);

	template < typename ObjectType >
	ObjectType* getObject() const
	{
		return dynamic_type_cast< ObjectType* >(getObject());
	}

	template < typename ObjectType >
	Ref< ObjectType > getObjectSafe() const
	{
		return dynamic_type_cast< ObjectType* >(getObjectSafe());
	}

private:
	union Value
	{
		bool b;
		avm_number_t n;
		char* s;
		ActionObject* o;
	};

	Type m_type;
	Value m_value;
};

	}
}

#endif	// traktor_flash_ActionValue_H
