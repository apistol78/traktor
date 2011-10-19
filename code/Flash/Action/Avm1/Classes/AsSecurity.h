#ifndef traktor_flash_AsSecurity_H
#define traktor_flash_AsSecurity_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Security class.
 * \ingroup Flash
 */
class AsSecurity : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSecurity(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;

private:
	void Security_allowDomain(CallArgs& ca);

	void Security_allowInsecureDomain(CallArgs& ca);

	void Security_loadPolicyFile(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSecurity_H
