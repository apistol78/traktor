#ifndef traktor_flash_AsKey_H
#define traktor_flash_AsKey_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionVM;
class ActionContext;
struct CallArgs;

/*! \brief Key class.
 * \ingroup Flash
 */
class AsKey : public ActionClass
{
	T_RTTI_CLASS(AsKey)

public:
	static AsKey* createInstance();

	AsKey();

	void eventKeyDown(ActionVM* actionVM, ActionContext* context, int keyCode);

	void eventKeyUp(ActionVM* actionVM, ActionContext* context, int keyCode);

private:
	RefArray< ActionObject > m_listeners;
	bool m_keyState[256];
	int m_lastKeyCode;

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Key_addListener(CallArgs& ca);

	void Key_getAscii(CallArgs& ca);

	void Key_getCode(CallArgs& ca);

	void Key_isAccessible(CallArgs& ca);

	void Key_isDown(CallArgs& ca);

	void Key_isToggled(CallArgs& ca);

	void Key_removeListener(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsKey_H
