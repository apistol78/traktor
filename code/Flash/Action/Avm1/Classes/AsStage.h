#ifndef traktor_flash_AsStage_H
#define traktor_flash_AsStage_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Stage class.
 * \ingroup Flash
 */
class AsStage : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsStage();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void Stage_get_width(CallArgs& ca);

	void Stage_get_height(CallArgs& ca);

	void Stage_addListener(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsStage_H
