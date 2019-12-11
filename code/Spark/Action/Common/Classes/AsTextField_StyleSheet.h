#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! TextField.StyleSheet class.
 * \ingroup Spark
 */
class AsTextField_StyleSheet : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextField_StyleSheet(ActionContext* context);

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual void initialize(ActionObject* self) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void TextField_StyleSheet_clear(CallArgs& ca);

	void TextField_StyleSheet_getStyle(CallArgs& ca);

	void TextField_StyleSheet_getStyleNames(CallArgs& ca);

	void TextField_StyleSheet_load(CallArgs& ca);

	void TextField_StyleSheet_parseCSS(CallArgs& ca);

	void TextField_StyleSheet_setStyle(CallArgs& ca);

	void TextField_StyleSheet_transform(CallArgs& ca);
};

	}
}

