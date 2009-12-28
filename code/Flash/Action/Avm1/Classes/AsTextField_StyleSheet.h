#ifndef traktor_flash_AsTextField_StyleSheet_H
#define traktor_flash_AsTextField_StyleSheet_H

#include "Flash/Action/Avm1/ActionClass.h"

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
	static Ref< AsTextField_StyleSheet > getInstance();

private:
	AsTextField_StyleSheet();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

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
