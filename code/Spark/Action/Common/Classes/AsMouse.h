#pragma once

#include "Core/RefArray.h"
#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! Mouse class.
 * \ingroup Spark
 */
class AsMouse : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsMouse(ActionContext* context);

	void eventMouseDown(int x, int y, int button);

	void eventMouseUp(int x, int y, int button);

	void eventMouseMove(int x, int y, int button);

	void eventMouseWheel(int x, int y, int delta);

private:
	void Mouse_show(CallArgs& ca);

	void Mouse_hide(CallArgs& ca);
};

	}
}

