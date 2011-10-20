#ifndef traktor_flash_AsBoolean_H
#define traktor_flash_AsBoolean_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

class Boolean;

/*! \brief Boolean class.
 * \ingroup Flash
 */
class AsBoolean : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsBoolean(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

private:
	std::string Boolean_toString(Boolean* self) const;

	bool Boolean_valueOf(Boolean* self) const;
};

	}
}

#endif	// traktor_flash_AsBoolean_H
