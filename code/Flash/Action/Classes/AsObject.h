#ifndef traktor_flash_AsObject_H
#define traktor_flash_AsObject_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Object class.
 * \ingroup Flash
 */
class AsObject : public ActionClass
{
	T_RTTI_CLASS(AsObject)

public:
	static AsObject* getInstance();

private:
	AsObject();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Object_addProperty(CallArgs& ca);

	void Object_hasOwnProperty(CallArgs& ca);

	void Object_isPropertyEnumerable(CallArgs& ca);

	void Object_isPrototypeOf(CallArgs& ca);

	void Object_registerClass(CallArgs& ca);

	void Object_toString(CallArgs& ca);

	void Object_unwatch(CallArgs& ca);

	void Object_valueOf(CallArgs& ca);

	void Object_watch(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsObject_H
