#ifndef traktor_flash_IActionObjectRelay_H
#define traktor_flash_IActionObjectRelay_H

#include "Flash/Collectable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionObject;
class ActionValue;

class T_DLLCLASS IActionObjectRelay : public Collectable
{
	T_RTTI_CLASS;

public:
	virtual void setAsObject(ActionObject* asObject) = 0;

	virtual ActionObject* getAsObject(ActionContext* context) = 0;

	virtual bool setMember(ActionContext* context, const std::string& memberName, const ActionValue& memberValue) = 0;

	virtual bool getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue) = 0;
};

	}
}

#endif	// traktor_flash_IActionObjectRelay_H
