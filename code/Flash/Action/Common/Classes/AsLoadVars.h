#pragma once

#include "Flash/Action/ActionClass.h"

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
	AsLoadVars(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
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

