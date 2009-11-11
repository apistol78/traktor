#ifndef traktor_flash_AsSecurity_H
#define traktor_flash_AsSecurity_H

#include "Flash/Action/ActionClass.h"

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
	T_RTTI_CLASS(AsSecurity)

public:
	static Ref< AsSecurity > getInstance();

private:
	AsSecurity();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Security_allowDomain(CallArgs& ca);

	void Security_allowInsecureDomain(CallArgs& ca);

	void Security_loadPolicyFile(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSecurity_H
