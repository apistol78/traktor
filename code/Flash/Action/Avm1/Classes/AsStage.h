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
	static Ref< AsStage > getInstance();

private:
	AsStage();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Stage_get_width(CallArgs& ca);

	void Stage_set_width(CallArgs& ca);

	void Stage_get_height(CallArgs& ca);

	void Stage_set_height(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsStage_H
