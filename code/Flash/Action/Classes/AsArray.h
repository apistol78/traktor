#ifndef traktor_flash_AsArray_H
#define traktor_flash_AsArray_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Array class.
 * \ingroup Flash
 */
class AsArray : public ActionClass
{
	T_RTTI_CLASS(AsArray)

public:
	static AsArray* getInstance();

private:
	AsArray();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Array_concat(CallArgs& ca);

	void Array_join(CallArgs& ca);

	void Array_pop(CallArgs& ca);

	void Array_push(CallArgs& ca);

	void Array_reverse(CallArgs& ca);

	void Array_shift(CallArgs& ca);

	void Array_slice(CallArgs& ca);

	void Array_sort(CallArgs& ca);

	void Array_sortOn(CallArgs& ca);

	void Array_splice(CallArgs& ca);

	void Array_toString(CallArgs& ca);

	void Array_unshift(CallArgs& ca);

	void Array_get_length(CallArgs& ca);

	void Array_set_length(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsArray_H
