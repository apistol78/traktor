#ifndef traktor_flash_AsMouse_H
#define traktor_flash_AsMouse_H

#include "Core/RefArray.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Mouse class.
 * \ingroup Flash
 */
class AsMouse : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsMouse(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

	void eventMouseDown(int x, int y, int button);

	void eventMouseUp(int x, int y, int button);

	void eventMouseMove(int x, int y, int button);

	void removeAllListeners();

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	RefArray< ActionObject > m_listeners;

	void Mouse_addListener(CallArgs& ca);

	void Mouse_removeListener(CallArgs& ca);

	void Mouse_show(CallArgs& ca);

	void Mouse_hide(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMouse_H
