#ifndef traktor_flash_AsError_H
#define traktor_flash_AsError_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Error class.
 * \ingroup Flash
 */
class AsError : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsError(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void Error_toString(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsError_H
