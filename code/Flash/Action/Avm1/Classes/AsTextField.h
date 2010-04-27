#ifndef traktor_flash_AsTextField_H
#define traktor_flash_AsTextField_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class FlashEditInstance;

/*! \brief TextField class.
 * \ingroup Flash
 */
class AsTextField : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsTextField > getInstance();

private:
	AsTextField();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	std::wstring TextField_get_text(FlashEditInstance* editInstance) const;

	void TextField_set_text(FlashEditInstance* editInstance, const std::wstring& text) const;

	avm_number_t TextField_get_textWidth(FlashEditInstance* editInstance) const;

	avm_number_t TextField_get_textHeight(FlashEditInstance* editInstance) const;
};

	}
}

#endif	// traktor_flash_AsTextField_H
