#ifndef traktor_flash_AsMouse_H
#define traktor_flash_AsMouse_H

#include "Core/RefArray.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionContext;
class IActionVM;
struct CallArgs;

/*! \brief Mouse class.
 * \ingroup Flash
 */
class AsMouse : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsMouse > createInstance();

	AsMouse();

	void eventMouseDown(IActionVM* actionVM, ActionContext* context, int x, int y, int button);

	void eventMouseUp(IActionVM* actionVM, ActionContext* context, int x, int y, int button);

	void eventMouseMove(IActionVM* actionVM, ActionContext* context, int x, int y, int button);

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
