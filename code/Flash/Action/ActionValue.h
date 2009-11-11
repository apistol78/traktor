#ifndef traktor_flash_ActionValue_H
#define traktor_flash_ActionValue_H

#include "Core/Heap/Ref.h"
#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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

	explicit ActionValue(double n);

	explicit ActionValue(const wchar_t* s);

	explicit ActionValue(const std::wstring& s);

	virtual ~ActionValue();

	/*! \brief Wrap object as value. */
	static ActionValue fromObject(ActionObject* const o);

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
	double getNumber() const { T_ASSERT_M (m_type == AvtNumber, L"Incorrect type"); return m_value.n; }

	/*! \brief Get string value. */
	std::wstring getString() const { T_ASSERT_M (m_type == AvtString, L"Incorrect type"); return m_value.s; }

	/*! \brief Get object value. */
	Ref< ActionObject > getObject() const { T_ASSERT_M (m_type == AvtObject, L"Incorrect type"); return *m_value.o; }

	/*! \brief Get boolean value safe. */
	bool getBooleanSafe() const;

	/*! \brief Get number value safe. */
	double getNumberSafe() const;

	/*! \brief Get string value safe. */
	std::wstring getStringSafe() const;

	/*! \brief Get object safe. */
	Ref< ActionObject > getObjectSafe() const;

	ActionValue& operator = (const ActionValue& v);

	template < typename ObjectType >
	Ref< ObjectType > getObject() const
	{
		return dynamic_type_cast< ObjectType* >(getObject());
	}

private:
	union Value
	{
		bool b;
		double n;
		wchar_t* s;
		Ref< ActionObject >* o;
	};

	Type m_type;
	Value m_value;
};

	}
}

#endif	// traktor_flash_ActionValue_H
