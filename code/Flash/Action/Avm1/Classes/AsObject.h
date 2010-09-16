#ifndef traktor_flash_AsObject_H
#define traktor_flash_AsObject_H

#include "Flash/Action/Avm1/ActionClass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Object class.
 * \ingroup Flash
 */
class T_DLLCLASS AsObject : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsObject();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

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
