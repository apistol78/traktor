#ifndef traktor_flash_AsKey_H
#define traktor_flash_AsKey_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Key class.
 * \ingroup Flash
 */
class AsKey : public ActionObject
{
	T_RTTI_CLASS;

public:
	enum AsKeyEnum
	{
		AkBackspace = 8,
		AkCapsLock = 20,
		AkControl = 17,
		AkDeleteKey = 46,
		AkDown = 40,
		AkEnd = 35,
		AkEnter = 13,
		AkEscape = 27,
		AkHome = 36,
		AkInsert = 45,
		AkLeft = 37,
		AkPgDn = 34,
		AkPgUp = 33,
		AkRight = 39,
		AkShift = 16,
		AkSpace = 32,
		AkTab = 9,
		AkUp = 38
	};

	AsKey(ActionContext* context);

	void eventKeyDown(int keyCode);

	void eventKeyUp(int keyCode);

private:
	bool m_keyState[256];
	int m_lastKeyCode;

	void Key_getAscii(CallArgs& ca);

	void Key_getCode(CallArgs& ca);

	void Key_isAccessible(CallArgs& ca);

	void Key_isDown(CallArgs& ca);

	void Key_isToggled(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsKey_H
