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
	T_RTTI_CLASS(AsError)

public:
	static Ref< AsError > getInstance();

private:
	AsError();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);
};

	}
}

#endif	// traktor_flash_AsError_H
