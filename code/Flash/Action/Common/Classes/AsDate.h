#ifndef traktor_flash_AsDate_H
#define traktor_flash_AsDate_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Date;

/*! \brief Date class.
 * \ingroup Flash
 */
class AsDate : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsDate(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void Date_getDate(const Date* self) const;

	void Date_getDay(const Date* self) const;

	void Date_getFullYear(const Date* self) const;

	void Date_getHours(const Date* self) const;

	void Date_getMilliseconds(const Date* self) const;

	void Date_getMinutes(const Date* self) const;

	void Date_getMonth(const Date* self) const;

	void Date_getSeconds(const Date* self) const;

	void Date_getTime(const Date* self) const;

	void Date_getTimezoneOffset(const Date* self) const;

	void Date_getUTCDate(const Date* self) const;

	void Date_getUTCDay(const Date* self) const;

	void Date_getUTCFullYear(const Date* self) const;

	void Date_getUTCHours(const Date* self) const;

	void Date_getUTCMilliseconds(const Date* self) const;

	void Date_getUTCMinutes(const Date* self) const;

	void Date_getUTCMonth(const Date* self) const;

	void Date_getUTCSeconds(const Date* self) const;

	void Date_getUTCYear(const Date* self) const;

	void Date_getYear(const Date* self) const;

	void Date_setDate(Date* self) const;

	void Date_setFullYear(Date* self) const;

	void Date_setHours(Date* self) const;

	void Date_setMilliseconds(Date* self) const;

	void Date_setMinutes(Date* self) const;

	void Date_setMonth(Date* self) const;

	void Date_setSeconds(Date* self) const;

	void Date_setTime(Date* self) const;

	void Date_setUTCDate(Date* self) const;

	void Date_setUTCFullYear(Date* self) const;

	void Date_setUTCHours(Date* self) const;

	void Date_setUTCMilliseconds(Date* self) const;

	void Date_setUTCMinutes(Date* self) const;

	void Date_setUTCMonth(Date* self) const;

	void Date_setUTCSeconds(Date* self) const;

	void Date_setYear(Date* self) const;

	std::string Date_toString(Date* self) const;

	void Date_UTC(Date* self) const;

	void Date_valueOf(Date* self) const;
};

	}
}

#endif	// traktor_flash_AsDate_H
