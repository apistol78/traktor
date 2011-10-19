#ifndef traktor_flash_AsArray_H
#define traktor_flash_AsArray_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Array;
struct CallArgs;

/*! \brief Array class.
 * \ingroup Flash
 */
class AsArray : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsArray(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;

private:
	void Array_concat(CallArgs& ca);

	void Array_join(CallArgs& ca);

	void Array_reverse(CallArgs& ca);

	void Array_shift(CallArgs& ca);

	void Array_slice(CallArgs& ca);

	void Array_sort(CallArgs& ca);

	void Array_sortOn(CallArgs& ca);

	void Array_splice(CallArgs& ca);

	ActionValue Array_toString(const Array* self) const;

	void Array_unshift(CallArgs& ca);

	uint32_t Array_get_length(const Array* self) const;
};

	}
}

#endif	// traktor_flash_AsArray_H
