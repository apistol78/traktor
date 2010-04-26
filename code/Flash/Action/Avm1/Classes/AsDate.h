#ifndef traktor_flash_AsDate_H
#define traktor_flash_AsDate_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Date class.
 * \ingroup Flash
 */
class AsDate : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsDate > getInstance();

private:
	AsDate();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Date_getDate(CallArgs& ca);

	void Date_getDay(CallArgs& ca);

	void Date_getFullYear(CallArgs& ca);

	void Date_getHours(CallArgs& ca);

	void Date_getMilliseconds(CallArgs& ca);

	void Date_getMinutes(CallArgs& ca);

	void Date_getMonth(CallArgs& ca);

	void Date_getSeconds(CallArgs& ca);

	void Date_getTime(CallArgs& ca);

	void Date_getTimezoneOffset(CallArgs& ca);

	void Date_getUTCDate(CallArgs& ca);

	void Date_getUTCDay(CallArgs& ca);

	void Date_getUTCFullYear(CallArgs& ca);

	void Date_getUTCHours(CallArgs& ca);

	void Date_getUTCMilliseconds(CallArgs& ca);

	void Date_getUTCMinutes(CallArgs& ca);

	void Date_getUTCMonth(CallArgs& ca);

	void Date_getUTCSeconds(CallArgs& ca);

	void Date_getUTCYear(CallArgs& ca);

	void Date_getYear(CallArgs& ca);

	void Date_setDate(CallArgs& ca);

	void Date_setFullYear(CallArgs& ca);

	void Date_setHours(CallArgs& ca);

	void Date_setMilliseconds(CallArgs& ca);

	void Date_setMinutes(CallArgs& ca);

	void Date_setMonth(CallArgs& ca);

	void Date_setSeconds(CallArgs& ca);

	void Date_setTime(CallArgs& ca);

	void Date_setUTCDate(CallArgs& ca);

	void Date_setUTCFullYear(CallArgs& ca);

	void Date_setUTCHours(CallArgs& ca);

	void Date_setUTCMilliseconds(CallArgs& ca);

	void Date_setUTCMinutes(CallArgs& ca);

	void Date_setUTCMonth(CallArgs& ca);

	void Date_setUTCSeconds(CallArgs& ca);

	void Date_setYear(CallArgs& ca);

	void Date_toString(CallArgs& ca);

	void Date_UTC(CallArgs& ca);

	void Date_valueOf(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsDate_H
