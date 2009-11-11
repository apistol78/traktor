#ifndef traktor_flash_AsLocalConnection_H
#define traktor_flash_AsLocalConnection_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief LocalConnection class.
 * \ingroup Flash
 */
class AsLocalConnection : public ActionClass
{
	T_RTTI_CLASS(AsLocalConnection)

public:
	static Ref< AsLocalConnection > getInstance();

private:
	AsLocalConnection();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void LocalConnection_close(CallArgs& ca);

	void LocalConnection_connect(CallArgs& ca);

	void LocalConnection_domain(CallArgs& ca);

	void LocalConnection_send(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsLocalConnection_H
