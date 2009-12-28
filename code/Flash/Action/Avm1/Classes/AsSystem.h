#ifndef traktor_flash_AsSystem_H
#define traktor_flash_AsSystem_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief System class.
 * \ingroup Flash
 */
class AsSystem : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsSystem > getInstance();

private:
	AsSystem();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);
};

	}
}

#endif	// traktor_flash_AsSystem_H
