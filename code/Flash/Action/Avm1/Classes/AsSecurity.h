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
	AsSecurity();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Security_allowDomain(CallArgs& ca);

	void Security_allowInsecureDomain(CallArgs& ca);

	void Security_loadPolicyFile(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSecurity_H
