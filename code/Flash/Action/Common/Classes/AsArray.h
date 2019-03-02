#pragma once

#include "Flash/Action/ActionClass.h"

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

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

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

	void Array_set_length(Array* self, uint32_t length) const;
};

	}
}

