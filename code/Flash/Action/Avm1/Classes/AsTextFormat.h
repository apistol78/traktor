#ifndef traktor_flash_AsTextFormat_H
#define traktor_flash_AsTextFormat_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class TextFormat;

/*! \brief TextFormat class.
 * \ingroup Flash
 */
class AsTextFormat : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsTextFormat(ActionContext* context);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual void initialize(ActionObject* self);

	virtual ActionValue xplicit(const ActionValueArray& args);
};

	}
}

#endif	// traktor_flash_AsTextFormat_H
