#ifndef traktor_flash_AsStage_H
#define traktor_flash_AsStage_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Stage class.
 * \ingroup Flash
 */
class AsStage : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsStage(ActionContext* context);

private:
	void Stage_get_width(CallArgs& ca);

	void Stage_get_height(CallArgs& ca);

	void Stage_addListener(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsStage_H
