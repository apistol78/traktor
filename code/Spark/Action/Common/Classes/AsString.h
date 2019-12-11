#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;
class String;

/*! String class.
 * \ingroup Spark
 */
class AsString : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsString(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void String_fromCharCode(CallArgs& ca);

	std::string String_charAt(const String* self, uint32_t index) const;

	uint32_t String_charCodeAt(const String* self, uint32_t index) const;

	void String_concat(CallArgs& ca);

	int32_t String_indexOf(const String* self, const std::string& needle) const;

	int32_t String_lastIndexOf(const String* self, const std::string& needle) const;

	void String_slice(CallArgs& ca);

	void String_split(CallArgs& ca);

	void String_substr(CallArgs& ca);

	void String_substring(CallArgs& ca);

	void String_toLowerCase(CallArgs& ca);

	void String_toString(CallArgs& ca);

	void String_toUpperCase(CallArgs& ca);

	void String_valueOf(CallArgs& ca);

	int32_t String_get_length(const String* self) const;
};

	}
}

