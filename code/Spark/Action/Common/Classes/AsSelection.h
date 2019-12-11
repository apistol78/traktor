#pragma once

#include "Core/RefArray.h"
#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! Selection class.
 * \ingroup Spark
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

