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
	AsTextFormat();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);
};

	}
}

#endif	// traktor_flash_AsTextFormat_H
