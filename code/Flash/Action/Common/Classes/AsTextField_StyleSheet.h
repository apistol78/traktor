#ifndef traktor_flash_AsTextField_StyleSheet_H
#define traktor_flash_AsTextField_StyleSheet_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief TextField.StyleSheet class.
 * \ingroup Flash
 */
class AsTextField_StyleSheet : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextField_StyleSheet(ActionContext* context);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual void initialize(ActionObject* self);

	virtual ActionValue xplicit(const ActionValueArray& args);

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

#endif	// traktor_flash_AsTextField_StyleSheet_H
