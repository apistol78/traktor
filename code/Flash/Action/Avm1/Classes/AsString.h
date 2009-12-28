#ifndef traktor_flash_AsString_H
#define traktor_flash_AsString_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief String class.
 * \ingroup Flash
 */
class AsString : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsString > getInstance();

private:
	AsString();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void String_charAt(CallArgs& ca);

	void String_charCodeAt(CallArgs& ca);

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
