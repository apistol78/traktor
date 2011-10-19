#ifndef traktor_flash_AsError_H
#define traktor_flash_AsError_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Error class.
 * \ingroup Flash
 */
class AsError : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsError(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;
};

	}
}

#endif	// traktor_flash_AsError_H
