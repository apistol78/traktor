#ifndef traktor_flash_AsLoadVars_H
#define traktor_flash_AsLoadVars_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief LoadVars class.
 * \ingroup Flash
 */
class AsLoadVars : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsLoadVars();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void LoadVars_addRequestHeader(CallArgs& ca);

	void LoadVars_decode(CallArgs& ca);

	void LoadVars_getBytesLoaded(CallArgs& ca);

	void LoadVars_getBytesTotal(CallArgs& ca);

	void LoadVars_load(CallArgs& ca);

	void LoadVars_send(CallArgs& ca);

	void LoadVars_sendAndLoad(CallArgs& ca);

	void LoadVars_toString(CallArgs& ca);

	void LoadVars_get_contentType(CallArgs& ca);

	void LoadVars_set_contentType(CallArgs& ca);

	void LoadVars_get_loaded(CallArgs& ca);

	void LoadVars_set_loaded(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsLoadVars_H
