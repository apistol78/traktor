#ifndef traktor_flash_AsMouse_H
#define traktor_flash_AsMouse_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Mouse class.
 * \ingroup Flash
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

#endif	// traktor_flash_AsMouse_H
