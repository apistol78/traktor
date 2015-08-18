#ifndef traktor_flash_AsSelection_H
#define traktor_flash_AsSelection_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Selection class.
 * \ingroup Flash
 */
class AsSelection : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsSelection(ActionContext* context);

private:
	void Selection_getBeginIndex(CallArgs& ca);

	void Selection_getCaretIndex(CallArgs& ca);

	void Selection_getEndIndex(CallArgs& ca);

	void Selection_getFocus(CallArgs& ca);

	void Selection_setFocus(CallArgs& ca);

	void Selection_setSelection(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSelection_H
