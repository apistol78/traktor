#ifndef traktor_flash_AsMouse_H
#define traktor_flash_AsMouse_H

#include "Core/Heap/RefArray.h"
#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionVM;
class ActionContext;
struct CallArgs;

/*! \brief Mouse class.
 * \ingroup Flash
 */
class AsMouse : public ActionClass
{
	T_RTTI_CLASS(AsMouse)

public:
	static Ref< AsMouse > createInstance();

	AsMouse();

	void eventMouseDown(ActionVM* actionVM, ActionContext* context, int x, int y, int button);

	void eventMouseUp(ActionVM* actionVM, ActionContext* context, int x, int y, int button);

	void eventMouseMove(ActionVM* actionVM, ActionContext* context, int x, int y, int button);

private:
	RefArray< ActionObject > m_listeners;

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Mouse_addListener(CallArgs& ca);

	void Mouse_removeListener(CallArgs& ca);

	void Mouse_show(CallArgs& ca);

	void Mouse_hide(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMouse_H
