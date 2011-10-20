#ifndef traktor_flash_AsString_H
#define traktor_flash_AsString_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class String;

/*! \brief String class.
 * \ingroup Flash
 */
class AsString : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsString(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

private:
	Ref< String > String_charAt(const String* self, uint32_t index) const;

	uint32_t String_charCodeAt(const String* self, uint32_t index) const;

	void String_concat(CallArgs& ca);

	void String_fromCharCode(CallArgs& ca);

	void String_indexOf(CallArgs& ca);

	void String_lastIndexOf(CallArgs& ca);

	void String_slice(CallArgs& ca);

	void String_split(CallArgs& ca);

	void String_substr(CallArgs& ca);

	void String_substring(CallArgs& ca);

	void String_toLowerCase(CallArgs& ca);

	void String_toString(CallArgs& ca);

	void String_toUpperCase(CallArgs& ca);

	void String_valueOf(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsString_H
