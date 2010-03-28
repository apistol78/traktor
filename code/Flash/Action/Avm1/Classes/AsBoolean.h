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
	static Ref< AsBoolean > getInstance();

private:
	AsBoolean();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	std::wstring Boolean_toString(Boolean* self) const;

	bool Boolean_valueOf(Boolean* self) const;
};

	}
}

#endif	// traktor_flash_AsBoolean_H
